#pragma once

#include "web/handlers/webhandlers.h"

#include <memory>
#include <vector>

namespace uPocketWS::Web {
/**
     * @brief WebRequestDispatcher Routes a request to the first matching handler.
     *
     * Two phases:
     *  1. Action handlers (?action=upload/delete/mkdir): matched on method + query,
     *     no filesystem resolution needed.
     *  2. Resource handlers (exec/static/targz/index): the requested path is
     *     resolved ONCE into ctx.fileInfo, then handlers decide on that info.
     * When nothing matches, a 404 page is returned.
     */
class WebRequestDispatcher
{
public:
    WebRequestDispatcher();

    Handlers::IWebRequestHandler::Code dispatch(WebClientHdlr &srv, WebRequestContext &ctx);

private:
    /** @brief resolveResource Resolve the URI (with the index.html fallback chain) into ctx.fileInfo. */
    static bool resolveResource(WebClientHdlr &srv, WebRequestContext &ctx);

    std::vector<std::unique_ptr<Handlers::IWebRequestHandler>> m_actionHandlers;
    std::vector<std::unique_ptr<Handlers::IWebRequestHandler>> m_resourceHandlers;
};
} // namespace uPocketWS::Web
