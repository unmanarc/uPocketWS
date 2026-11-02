#include "web/webrequestdispatcher.h"
#include "web/view/html/html_errors.h"
#include "web/webclienthdlr.h"

namespace uPocketWS::Web {
using namespace Handlers;

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
    for (const char *suffix : {".html", "index.html", "/index.html", ""})
    {
        if (srv.resolveLocalFilePathFromURI2(cfg.httpDocumentRootDir, {}, &ctx.fileInfo, suffix, false) && !ctx.fileInfo.isDirectory)
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
