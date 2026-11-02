#include "web/webrequestdispatcher.h"
#include "web/handlers/webhandlers.h"
#include "web/view/html/html_errors.h"
#include "web/webclienthdlr.h"

#include <filesystem>

namespace uPocketWS::Web {
using namespace Handlers;
namespace fs = std::filesystem;

WebRequestDispatcher::WebRequestDispatcher()
{
    // Phase 1: ?action= endpoints (order is irrelevant, they are mutually exclusive).
    m_actionHandlers.push_back(std::make_unique<UploadHandler>());
    m_actionHandlers.push_back(std::make_unique<DeleteHandler>());
    m_actionHandlers.push_back(std::make_unique<MkdirHandler>());

    // Phase 2: resolved resources (order matters: exec before static, targz before index).
    m_resourceHandlers.push_back(std::make_unique<ExecHandler>());
    m_resourceHandlers.push_back(std::make_unique<StaticHandler>());
    m_resourceHandlers.push_back(std::make_unique<TarGzHandler>());
    m_resourceHandlers.push_back(std::make_unique<IndexHandler>());
}

bool WebRequestDispatcher::resolveResource(WebClientHdlr &srv, WebRequestContext &ctx)
{
    const webClientParams &cfg = srv.params();

    // Files first: "x" -> "x.html", "dir/" -> "dir/index.html", "dir" -> "dir/index.html", then "x".
    // preventMappingExecutables is passed only when the resolved file lies inside a
    // configured execute-dir, so executables there are mapped with isExecutable=true
    // and routed to ExecHandler (CGI-like), instead of being served as static content.
    for (const char *suffix : {".html", "index.html", "/index.html", ""})
    {
        bool insideExecDir = false;
        if (!cfg.executeDirs.empty())
        {
            // The URI is absolute ("/x/y"): strip the leading slashes, otherwise
            // operator/ would discard the document root and yield "/x/y".
            std::string relativeURI = srv.clientRequest.getURI() + suffix;
            relativeURI.erase(0, relativeURI.find_first_not_of('/'));

            std::error_code ec;
            const fs::path canonicalFile = fs::canonical(fs::path(cfg.httpDocumentRootDir) / relativeURI, ec);
            insideExecDir = !ec && fs::is_regular_file(canonicalFile, ec) && !ec && isPathInAnyDirList(cfg.executeDirs, canonicalFile.parent_path().string());
        }
        if (srv.resolveLocalFilePathFromURI2(cfg.httpDocumentRootDir, {}, &ctx.fileInfo, suffix, insideExecDir) && !ctx.fileInfo.isDirectory)
            return true;
    }

    // Then directories (index / tar.gz).
    if (srv.resolveLocalFilePathFromURI2(cfg.httpDocumentRootDir, {}, &ctx.fileInfo) && ctx.fileInfo.isDirectory)
    {
        if (ctx.fileInfo.relativePath.empty())
            ctx.fileInfo.relativePath = "/";
        return true;
    }

    ctx.fileInfo.reset();
    return false;
}

IWebRequestHandler::Code WebRequestDispatcher::dispatch(WebClientHdlr &srv, WebRequestContext &ctx)
{
    for (auto &h : m_actionHandlers)
    {
        if (h->canHandle(srv, ctx))
            return h->handle(srv, ctx);
    }

    if (resolveResource(srv, ctx))
    {
        for (auto &h : m_resourceHandlers)
        {
            if (h->canHandle(srv, ctx))
                return h->handle(srv, ctx);
        }
    }

    srv.writeBody(HTML_404);
    return srv.respond(Mantids30::Network::Protocol::HTTP::Status::Code::S_404_NOT_FOUND, ctx.fileInfo.relativePath.empty() ? srv.clientRequest.getURI() : ctx.fileInfo.relativePath);
}
} // namespace uPocketWS::Web
