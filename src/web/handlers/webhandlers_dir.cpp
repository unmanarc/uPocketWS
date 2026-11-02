#include "utils/webutils.h"
#include "web/handlers/webhandlers.h"
#include "web/view/viewindex.h"
#include "web/webclienthdlr.h"
#include "web/weblog.h"

#include <Mantids30/Helpers/appexec.h>
#include <Mantids30/Memory/streamable_process.h>

#include <filesystem>

using namespace Mantids30;
using namespace Mantids30::Network::Protocol;
using namespace Mantids30::Program;
namespace fs = std::filesystem;

namespace uPocketWS::Web::Handlers {
///////////////////////////////////////////////////////////////////////////
// STATIC: the resolver already mapped the file as the response content.
bool StaticHandler::canHandle(WebClientHdlr &, const WebRequestContext &ctx) const
{
    return !ctx.fileInfo.isDirectory && !ctx.fileInfo.isExecutable;
}

IWebRequestHandler::Code StaticHandler::handle(WebClientHdlr &srv, WebRequestContext &ctx)
{
    const Code ret = HTTP::Status::Code::S_200_OK;
    Log::logRpc(srv.params().rpcLog, Logs::DEBUG, srv.remoteIP(), Log::SHORT_SIZE, "D/LOCAL,%03d: %s", static_cast<int>(ret), ctx.fileInfo.fullPath.c_str());
    return srv.respond(ret, ctx.fileInfo.relativePath);
}

///////////////////////////////////////////////////////////////////////////
// TAR.GZ: GET <dir>/?action=targz
bool TarGzHandler::canHandle(WebClientHdlr &srv, const WebRequestContext &ctx) const
{
    if (ctx.fileInfo.isDirectory && srv.getVar("action") == "targz" && !srv.params().archiveDirs.empty())
    {
        std::error_code ec;
        const fs::path canonicalDir = fs::canonical(ctx.fileInfo.fullPath, ec);
        return !ec && isPathInAnyDirList(srv.params().archiveDirs, canonicalDir.string());
    }
    return false;
}

IWebRequestHandler::Code TarGzHandler::handle(WebClientHdlr &srv, WebRequestContext &ctx)
{
    const webClientParams &cfg = srv.params();
    const auto &fileInfo = ctx.fileInfo;
    const Code ret = HTTP::Status::Code::S_200_OK;

    Log::logRpc(cfg.rpcLog, Logs::INFO, srv.remoteIP(), Log::SHORT_SIZE, "A/TARGZ-DIR,%03d: %s", static_cast<int>(ret), fileInfo.relativePath.c_str());

    srv.serverResponse.contentType = "application/octet-stream";

    // Download name derived from the directory name (sanitized).
    std::string dirName = fs::path(fileInfo.fullPath).filename().string();
    if (dirName.empty())
        dirName = fs::path(fileInfo.fullPath).parent_path().filename().string(); // path with ending slash
    std::string fileName = Utils::sanitizeDownloadFileName(dirName) + ".tar.gz";
    if (dirName.empty())
        fileName = "archive.tar.gz";
    srv.serverResponse.headers.add("Content-Disposition", "attachment; filename=\"" + fileName + "\"");

    auto *spawner = new Helpers::AppSpawn();
    spawner->setExec("/usr/bin/tar");
    spawner->addArgument("czf");
    spawner->addArgument("-");           // to stdout
    spawner->addArgument("--directory"); // chdir: archive only relative paths (no FS layout leak)
    spawner->addArgument(fileInfo.fullPath);
    spawner->addArgument(".");

    if (spawner->spawnProcess(true, false))
    {
        srv.serverResponse.setContentDataStreamer(std::make_shared<Memory::Streams::StreamableProcess>(spawner));
    }
    else
    {
        Log::logRpc(cfg.rpcLog, Logs::WARNING, srv.remoteIP(), Log::SHORT_SIZE, "R/EXEC-FAIL: tar czf");
        delete spawner;
    }
    return srv.respond(ret, fileInfo.relativePath);
}

///////////////////////////////////////////////////////////////////////////
// INDEX: directory listing.
bool IndexHandler::canHandle(WebClientHdlr &, const WebRequestContext &ctx) const
{
    return ctx.fileInfo.isDirectory;
}

IWebRequestHandler::Code IndexHandler::handle(WebClientHdlr &srv, WebRequestContext &ctx)
{
    View::IndexView::render(srv, ctx.fileInfo);
    return srv.respond(HTTP::Status::Code::S_200_OK, ctx.fileInfo.relativePath);
}
} // namespace uPocketWS::Web::Handlers
