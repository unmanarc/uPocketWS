#include "utils/webutils.h"
#include "web/handlers/webhandlers.h"
#include "web/webclienthdlr.h"
#include "web/weblog.h"

#include <Mantids30/Memory/streamable_file.h>

#include <boost/algorithm/string/predicate.hpp>

#include <fcntl.h>
#include <filesystem>

using namespace Mantids30;
using namespace Mantids30::Network::Protocol;
using namespace Mantids30::Program;
namespace fs = std::filesystem;

namespace uPocketWS::Web::Handlers {
bool IWebRequestHandler::isPost(WebClientHdlr &srv)
{
    return srv.clientRequest.requestLine.getHTTPMethod() == "POST";
}

// Opens the destination file and redirects the MIME part content into it.
static void redirectPartToFile(WebRequestContext &ctx, const std::shared_ptr<MIME::MIME_PartMessage> &partMessage, const HTTP::HTTPv1_Server::LocalRequestedFileInfo &reqInfo, int flags,
                               const char *okTag, const char *failTag)
{
    auto file = std::make_shared<Memory::Streams::StreamableFile>();
    if (file->open(reqInfo.fullPath.c_str(), flags, 0600) >= 0)
    {
        partMessage->getContent()->replaceContentContainer(file);
        Log::logRpc(ctx.cfg->rpcLog, Logs::INFO, ctx.remoteIP, Log::FULL_SIZE, "%s: %s", okTag, reqInfo.relativePath.c_str());
        ctx.uploadOK = true;
    }
    else
    {
        Log::logRpc(ctx.cfg->rpcLog, Logs::ERROR, ctx.remoteIP, Log::FULL_SIZE, "%s: %s", failTag, reqInfo.relativePath.c_str());
    }
}

// MIME trampoline: called for every multipart header; obj is the WebRequestContext.
static void onMimePartHeaderReady(void *obj, const std::string &, const std::shared_ptr<MIME::MIME_PartMessage> &partMessage)
{
    WebRequestContext &ctx = *static_cast<WebRequestContext *>(obj);
    const webClientParams &cfg = *ctx.cfg;

    auto disposition = partMessage->getHeader()->getOptionByName("Content-Disposition");
    if (!disposition)
        return;
    std::string fileName = disposition->getSubVar("filename");

    // Enforce the configured filename regex (when any).
    if (cfg.uploadNameRe && !boost::regex_match(fileName, *cfg.uploadNameRe))
    {
        Log::logRpc(cfg.rpcLog, Logs::ERROR, ctx.remoteIP, Log::FULL_SIZE, "R/UPLOAD-FAIL-NAMERE: %s", fileName.c_str());
        return;
    }

    HTTP::HTTPv1_Server::LocalRequestedFileInfo reqInfo;

    if (HTTP::HTTPv1_Server::resolveLocalFilePathFromURI0E("/" + fileName, ctx.uploadDirBase, &reqInfo))
    {
        // Existing file.
        if (!cfg.overwrite)
        {
            Log::logRpc(cfg.rpcLog, Logs::ERROR, ctx.remoteIP, Log::FULL_SIZE, "R/UPLOAD-FAIL-OVERWRITE: %s", reqInfo.relativePath.c_str());
            return;
        }
        redirectPartToFile(ctx, partMessage, reqInfo, O_WRONLY | O_CREAT | O_TRUNC, "R/UPLOAD-OVERW-OK", "R/UPLOAD-FAIL-PERMS1");
    }
    else if (HTTP::HTTPv1_Server::resolveLocalFilePathFromURI0NE("/" + fileName, ctx.uploadDirBase, &reqInfo) && !reqInfo.isTransversal)
    {
        // New file.
        redirectPartToFile(ctx, partMessage, reqInfo, O_WRONLY | O_CREAT, "R/UPLOAD-CREATE-OK", "R/UPLOAD-FAIL-PERMS2");
    }
    else if (reqInfo.isTransversal)
    {
        Log::logRpc(cfg.rpcLog, Logs::CRITICAL, ctx.remoteIP, Log::FULL_SIZE, "R/UPLOAD-TRANSVERSAL2: %s", fileName.c_str());
    }
    else
    {
        Log::logRpc(cfg.rpcLog, Logs::ERROR, ctx.remoteIP, Log::FULL_SIZE, "R/UPLOAD-FAIL: %s", fileName.c_str());
    }
}

bool UploadHandler::canHandle(WebClientHdlr &srv, const WebRequestContext &) const
{
    return !srv.params().uploadDirs.empty() && isPost(srv) && srv.getVar("action") == "upload";
}

bool UploadHandler::prepareFromClientHeader(WebClientHdlr &srv, WebRequestContext &ctx)
{
    const webClientParams &cfg = srv.params();
    const std::string uri = srv.clientRequest.getURI();

    // Single-site logger for this handler: (level, tag, arg) -> "TAG: arg".
    const auto logU = [&](Logs::LogLevel level, const char *tag, const char *arg) { Log::logRpc(cfg.rpcLog, level, ctx.remoteIP, Log::FULL_SIZE, "%s: %s", tag, arg); };

    if (srv.clientRequest.content.getContainerType() != HTTP::Content::ContainerType::MIME)
    {
        logU(Logs::ERROR, "R/UPLOAD-FAIL-NOTMIME", uri.c_str());
        return true; // answered with "false" by handle()
    }

    // Resolve the requested directory (docRoot + URI) to its real path once, then
    // accept it only if it is one of the configured upload dirs or a sub-dir of
    // one. Comparing canonical paths (instead of the raw strings) is what makes
    // the containment check robust against symlinks in the docroot.
    std::error_code ec;
    const fs::path requestedPath = fs::canonical(cfg.httpDocumentRootDir + uri, ec);
    if (ec)
    {
        // The target does not exist (or the URI contains '..' segments that
        // escape the docroot). An escape attempt is logged as transversal.
        const bool escape = boost::algorithm::contains(uri, "..");
        logU(escape ? Logs::CRITICAL : Logs::ERROR, escape ? "R/UPLOAD-TRANSVERSAL1" : "R/UPLOAD-FAIL-NOTALLOWED", uri.c_str());
        return false; // drop the connection
    }

    if (!fs::is_directory(requestedPath, ec) || ec)
    {
        logU(Logs::ERROR, "R/UPLOAD-FAIL-NOTALLOWED-NOTDIR", uri.c_str());
        return false;
    }

    // Containment: exact match, or the allowed dir followed by a '/' boundary.
    const auto isPathContained = [requested = requestedPath.string()](const std::string &allowed)
    { return requested == allowed || (boost::algorithm::starts_with(requested, allowed) && requested[allowed.size()] == '/'); };

    // uploadDirs are already canonical (resolved via fs::canonical at startup),
    // so compare directly without resolving again.
    for (const auto &allowedDir : cfg.uploadDirs)
    {
        if (isPathContained(allowedDir))
        {
            ctx.uploadDirBase = requestedPath.string(); // the actual requested dir (sub-dir allowed)
            srv.clientRequest.content.getMultiPartVars()->setCallbackOnHeaderReady(MIME::MIME_Message::MIMECallback(&onMimePartHeaderReady, &ctx));
            logU(Logs::INFO, "R/UPLOAD-REQ", uri.c_str());
            return true;
        }
    }

    logU(Logs::ERROR, "R/UPLOAD-FAIL-DIR-NOTALLOWED", uri.c_str());
    return false;
}

IWebRequestHandler::Code UploadHandler::handle(WebClientHdlr &srv, WebRequestContext &ctx)
{
    srv.writeBody(ctx.uploadOK ? "true" : "false");
    return srv.respond(HTTP::Status::Code::S_200_OK, srv.clientRequest.getURI());
}
} // namespace uPocketWS::Web::Handlers
