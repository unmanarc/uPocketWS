#include "utils/webutils.h"
#include "web/handlers/webhandlers.h"
#include "web/webclienthdlr.h"
#include "web/weblog.h"

#include <filesystem>

using namespace Mantids30::Network::Protocol;
using namespace Mantids30::Program;
namespace fs = std::filesystem;

namespace uPocketWS::Web::Handlers {
using FileInfo = HTTP::HTTPv1_Server::LocalRequestedFileInfo;

static IWebRequestHandler::Code fail(WebClientHdlr &srv, IWebRequestHandler::Code code)
{
    srv.writeBody("false");
    return srv.respond(code, srv.clientRequest.getURI());
}

/**
     * @brief resolveTargetDir Resolve the request URI as an existing directory
     *        inside the document root. fullPath is returned with an ending slash.
     * @return S_200_OK on success, otherwise the error code to answer.
     */
static IWebRequestHandler::Code resolveTargetDir(WebClientHdlr &srv, FileInfo &dirInfo, const char *tag)
{
    const webClientParams &cfg = srv.params();
    if (HTTP::HTTPv1_Server::resolveLocalFilePathFromURI0E(srv.clientRequest.getURI(), cfg.httpDocumentRootDir, &dirInfo) && !dirInfo.isTransversal)
    {
        if (!dirInfo.isDirectory)
            return HTTP::Status::Code::S_404_NOT_FOUND;
        if (!dirInfo.fullPath.empty() && dirInfo.fullPath.back() != '/')
            dirInfo.fullPath += '/';
        if (dirInfo.relativePath.empty() || dirInfo.relativePath.back() != '/')
            dirInfo.relativePath += '/';
        return HTTP::Status::Code::S_200_OK;
    }
    if (dirInfo.isTransversal)
    {
        Log::logRpc(cfg.rpcLog, Logs::CRITICAL, srv.remoteIP(), Log::FULL_SIZE, "R/%s-TRANSVERSAL: %s", tag, srv.clientRequest.getURI().c_str());
        return HTTP::Status::Code::S_403_FORBIDDEN;
    }
    return HTTP::Status::Code::S_404_NOT_FOUND;
}

///////////////////////////////////////////////////////////////////////////
// DELETE: POST <dir>/?action=delete&file=<name>
bool DeleteHandler::canHandle(WebClientHdlr &srv, const WebRequestContext &) const
{
    return srv.params().overwrite && isPost(srv) && srv.getVar("action") == "delete";
}

IWebRequestHandler::Code DeleteHandler::handle(WebClientHdlr &srv, WebRequestContext &)
{
    const webClientParams &cfg = srv.params();
    const std::string entryName = srv.getVar("file");

    if (entryName.empty())
        return fail(srv, HTTP::Status::Code::S_400_BAD_REQUEST);

    if (!Utils::isPlainFileName(entryName))
    {
        Log::logRpc(cfg.rpcLog, Logs::CRITICAL, srv.remoteIP(), Log::FULL_SIZE, "R/DELETE-BADNAME: %s", srv.clientRequest.getURI().c_str());
        return fail(srv, HTTP::Status::Code::S_403_FORBIDDEN);
    }

    FileInfo dirInfo;
    auto rc = resolveTargetDir(srv, dirInfo, "DELETE");
    if (rc != HTTP::Status::Code::S_200_OK)
        return fail(srv, rc);

    const std::string entryFull = dirInfo.fullPath + entryName;
    const std::string entryRel = dirInfo.relativePath + entryName;

    if (Utils::isProtectedMarkerFile(entryFull))
    {
        Log::logRpc(cfg.rpcLog, Logs::CRITICAL, srv.remoteIP(), Log::FULL_SIZE, "R/DELETE-NICETRY: %s", entryRel.c_str());
        return fail(srv, HTTP::Status::Code::S_403_FORBIDDEN);
    }

    std::error_code ec;
    if (!fs::exists(fs::symlink_status(entryFull, ec)) || ec)
        return fail(srv, HTTP::Status::Code::S_404_NOT_FOUND);

    if (fs::is_directory(fs::symlink_status(entryFull, ec)))
        fs::remove_all(entryFull, ec);
    else
        fs::remove(entryFull, ec);

    if (ec)
    {
        Log::logRpc(cfg.rpcLog, Logs::ERROR, srv.remoteIP(), Log::FULL_SIZE, "R/DELETE-FAIL: %s (%s)", entryRel.c_str(), ec.message().c_str());
        srv.writeBody("false");
        return srv.respond(HTTP::Status::Code::S_403_FORBIDDEN, entryRel);
    }

    Log::logRpc(cfg.rpcLog, Logs::INFO, srv.remoteIP(), Log::FULL_SIZE, "R/DELETE-OK: %s", entryRel.c_str());
    srv.writeBody("true");
    return srv.respond(HTTP::Status::Code::S_200_OK, entryRel);
}

///////////////////////////////////////////////////////////////////////////
// MKDIR: POST <dir>/?action=mkdir&name=<name>
bool MkdirHandler::canHandle(WebClientHdlr &srv, const WebRequestContext &) const
{
    return srv.params().overwrite && isPost(srv) && srv.getVar("action") == "mkdir";
}

IWebRequestHandler::Code MkdirHandler::handle(WebClientHdlr &srv, WebRequestContext &)
{
    const webClientParams &cfg = srv.params();
    const std::string dirName = srv.getVar("name");

    if (dirName.empty())
        return fail(srv, HTTP::Status::Code::S_400_BAD_REQUEST);

    FileInfo parentInfo;
    auto rc = resolveTargetDir(srv, parentInfo, "MKDIR");
    if (rc != HTTP::Status::Code::S_200_OK)
        return fail(srv, rc);

    // 0NE resolver: true -> does not exist and is inside the parent;
    // false + isTransversal -> escape attempt; false otherwise -> already exists.
    FileInfo newDirInfo;
    if (!HTTP::HTTPv1_Server::resolveLocalFilePathFromURI0NE("/" + dirName, parentInfo.fullPath, &newDirInfo))
    {
        if (newDirInfo.isTransversal)
        {
            Log::logRpc(cfg.rpcLog, Logs::CRITICAL, srv.remoteIP(), Log::FULL_SIZE, "R/MKDIR-TRANSVERSAL: %s", srv.clientRequest.getURI().c_str());
            return fail(srv, HTTP::Status::Code::S_403_FORBIDDEN);
        }
        return fail(srv, HTTP::Status::Code::S_409_CONFLICT);
    }

    std::error_code ec;
    if (fs::create_directory(newDirInfo.fullPath, ec))
    {
        Log::logRpc(cfg.rpcLog, Logs::INFO, srv.remoteIP(), Log::FULL_SIZE, "R/MKDIR-OK: %s", newDirInfo.fullPath.c_str());
        srv.writeBody("true");
        return srv.respond(HTTP::Status::Code::S_200_OK, srv.clientRequest.getURI());
    }

    Log::logRpc(cfg.rpcLog, Logs::ERROR, srv.remoteIP(), Log::FULL_SIZE, "R/MKDIR-FAIL: %s (%s)", newDirInfo.fullPath.c_str(), ec.message().c_str());
    return fail(srv, HTTP::Status::Code::S_403_FORBIDDEN);
}
} // namespace uPocketWS::Web::Handlers
