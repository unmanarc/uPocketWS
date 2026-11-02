#include "utils/webutils.h"
#include "web/handlers/webhandlers.h"
#include "web/webclienthdlr.h"
#include "web/weblog.h"

#include <Mantids30/Memory/streamable_file.h>

#include <fcntl.h>
#include <fstream>

using namespace Mantids30;
using namespace Mantids30::Network::Protocol;
using namespace Mantids30::Program;

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

    if (Utils::isRegularFile(ctx.uploadDirBase + "/.dontupload"))
    {
        Log::logRpc(cfg.rpcLog, Logs::ERROR, ctx.remoteIP, Log::FULL_SIZE, "R/UPLOAD-FAIL-PERMS-DIR: %s", ctx.uploadDirBase.c_str());
        return;
    }

    // Optional forced upload name.
    if (Utils::isRegularFile(ctx.uploadDirBase + "/.uploadname"))
    {
        std::ifstream file(ctx.uploadDirBase + "/.uploadname");
        if (!file)
            return;
        std::string line;
        if (std::getline(file, line))
        {
            while (!line.empty() && (line.back() == '\n' || line.back() == '\r'))
                line.pop_back();
            fileName = line;
        }
    }

    HTTP::HTTPv1_Server::LocalRequestedFileInfo reqInfo;

    if (HTTP::HTTPv1_Server::resolveLocalFilePathFromURI0E("/" + fileName, ctx.uploadDirBase, &reqInfo))
    {
        // Existing file.
        if (Utils::isProtectedMarkerFile(reqInfo.fullPath))
        {
            Log::logRpc(cfg.rpcLog, Logs::CRITICAL, ctx.remoteIP, Log::FULL_SIZE, "R/UPLOAD-NICETRY: %s", reqInfo.relativePath.c_str());
            return;
        }
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
        if (Utils::isProtectedMarkerFile(reqInfo.fullPath))
        {
            Log::logRpc(cfg.rpcLog, Logs::CRITICAL, ctx.remoteIP, Log::FULL_SIZE, "R/UPLOAD-NICETRY: %s", reqInfo.relativePath.c_str());
            return;
        }
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
    return srv.params().uploads && isPost(srv) && srv.getVar("action") == "upload";
}

bool UploadHandler::prepare(WebClientHdlr &srv, WebRequestContext &ctx)
{
    const webClientParams &cfg = srv.params();
    const std::string uri = srv.clientRequest.getURI();

    if (srv.clientRequest.content.getContainerType() != HTTP::Content::ContainerType::MIME)
    {
        Log::logRpc(cfg.rpcLog, Logs::ERROR, ctx.remoteIP, Log::FULL_SIZE, "R/UPLOAD-FAIL-NOTMIME: %s", uri.c_str());
        return true; // answered with "false" by handle()
    }

    HTTP::HTTPv1_Server::LocalRequestedFileInfo dirInfo;
    if (!HTTP::HTTPv1_Server::resolveLocalFilePathFromURI0E(uri, cfg.httpDocumentRootDir, &dirInfo))
    {
        Log::logRpc(cfg.rpcLog, Logs::CRITICAL, ctx.remoteIP, Log::FULL_SIZE, "R/UPLOAD-TRANSVERSAL1: %s", uri.c_str());
        return false;
    }
    if (!dirInfo.isDirectory)
    {
        Log::logRpc(cfg.rpcLog, Logs::ERROR, ctx.remoteIP, Log::FULL_SIZE, "R/UPLOAD-FAIL-NOTALLOWED: %s", uri.c_str());
        return false;
    }

    ctx.uploadDirBase = dirInfo.fullPath;
    srv.clientRequest.content.getMultiPartVars()->setCallbackOnHeaderReady(MIME::MIME_Message::MIMECallback(&onMimePartHeaderReady, &ctx));
    Log::logRpc(cfg.rpcLog, Logs::INFO, ctx.remoteIP, Log::FULL_SIZE, "R/UPLOAD-REQ: %s", uri.c_str());
    return true;
}

IWebRequestHandler::Code UploadHandler::handle(WebClientHdlr &srv, WebRequestContext &ctx)
{
    srv.writeBody(ctx.uploadOK ? "true" : "false");
    return srv.respond(HTTP::Status::Code::S_200_OK, srv.clientRequest.getURI());
}
} // namespace uPocketWS::Web::Handlers
