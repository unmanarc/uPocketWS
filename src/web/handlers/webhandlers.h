#pragma once

#include <Mantids30/Protocol_HTTP/rsp_status.h>

#include "web/webrequestcontext.h"

namespace uPocketWS::Web {
class WebClientHdlr;
}

namespace uPocketWS::Web::Handlers {
/**
     * @brief IWebRequestHandler Command interface: one handler per kind of request.
     *        The dispatcher asks handlers in order; the first canHandle() wins.
     */
class IWebRequestHandler
{
public:
    using Code = Mantids30::Network::Protocol::HTTP::Status::Code;

    virtual ~IWebRequestHandler() = default;

    virtual bool canHandle(WebClientHdlr &srv, const WebRequestContext &ctx) const = 0;
    virtual Code handle(WebClientHdlr &srv, WebRequestContext &ctx) = 0;

protected:
    static bool isPost(WebClientHdlr &srv);
};

/** @brief UploadHandler POST <dir>/?action=upload (multipart, field "file"). */
class UploadHandler : public IWebRequestHandler
{
public:
    bool canHandle(WebClientHdlr &srv, const WebRequestContext &ctx) const override;
    Code handle(WebClientHdlr &srv, WebRequestContext &ctx) override;

    /**
         * @brief prepareFromClientHeader Called when headers are received: resolves the target
         *        directory and installs the MIME callback that redirects each file
         *        part to disk. @return false to drop the connection.
         */
    static bool prepareFromClientHeader(WebClientHdlr &srv, WebRequestContext &ctx);
};

/** @brief DeleteHandler POST <dir>/?action=delete&file=<name>. */
class DeleteHandler : public IWebRequestHandler
{
public:
    bool canHandle(WebClientHdlr &srv, const WebRequestContext &ctx) const override;
    Code handle(WebClientHdlr &srv, WebRequestContext &ctx) override;
};

/** @brief MkdirHandler POST <dir>/?action=mkdir&name=<name>. */
class MkdirHandler : public IWebRequestHandler
{
public:
    bool canHandle(WebClientHdlr &srv, const WebRequestContext &ctx) const override;
    Code handle(WebClientHdlr &srv, WebRequestContext &ctx) override;
};

/** @brief ExecHandler CGI-like execution of executable files (--can-execute). */
class ExecHandler : public IWebRequestHandler
{
public:
    bool canHandle(WebClientHdlr &srv, const WebRequestContext &ctx) const override;
    Code handle(WebClientHdlr &srv, WebRequestContext &ctx) override;
};

/** @brief StaticHandler Regular file (already mapped as response by the resolver). */
class StaticHandler : public IWebRequestHandler
{
public:
    bool canHandle(WebClientHdlr &srv, const WebRequestContext &ctx) const override;
    Code handle(WebClientHdlr &srv, WebRequestContext &ctx) override;
};

/** @brief TarGzHandler GET <dir>/?action=targz. */
class TarGzHandler : public IWebRequestHandler
{
public:
    bool canHandle(WebClientHdlr &srv, const WebRequestContext &ctx) const override;
    Code handle(WebClientHdlr &srv, WebRequestContext &ctx) override;
};

/** @brief IndexHandler Directory listing. */
class IndexHandler : public IWebRequestHandler
{
public:
    bool canHandle(WebClientHdlr &srv, const WebRequestContext &ctx) const override;
    Code handle(WebClientHdlr &srv, WebRequestContext &ctx) override;
};
} // namespace uPocketWS::Web::Handlers
