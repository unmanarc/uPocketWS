#pragma once

#include <Mantids30/Protocol_HTTP/httpv1_server.h>

#include "web/webclientparams.h"
#include "web/webrequestcontext.h"
#include "web/webrequestdispatcher.h"

namespace uPocketWS::Web {
/**
     * @brief WebClientHdlr Thin HTTP connection handler: authentication/CSRF,
     *        upload preparation and delegation of every request to the dispatcher.
     */
class WebClientHdlr : public Mantids30::Network::Protocol::HTTP::HTTPv1_Server
{
public:
    using Code = Mantids30::Network::Protocol::HTTP::Status::Code;

    explicit WebClientHdlr(const std::shared_ptr<Mantids30::Memory::Streams::StreamableObject> &sock);

    void setWebClientParameters(const webClientParams &newWebClientParameters);
    const webClientParams &params() const { return m_params; }

    /** @brief respond Log the request result and return the code unchanged. */
    Code respond(Code ret, const std::string &uri);

    /** @brief remoteIP Remote client address of the current request. */
    std::string remoteIP() const { return clientRequest.networkClientInfo.REMOTE_ADDR; }

    /** @brief getVar Get a decoded GET query variable ("" when absent). */
    std::string getVar(const std::string &name);

    /** @brief writeBody Write a string to the response body. */
    void writeBody(const std::string &str);

    // Public wrapper for the protected base helper.
    bool detectContentType(const std::string &path) { return detectContentTypeFromFilePath(path); }

protected:
    bool onHTTPClientHeadersReceived() override;
    Code onHTTPClientContentReceived() override;

private:
    webClientParams m_params;
    WebRequestContext m_ctx;
    WebRequestDispatcher m_dispatcher;
};
} // namespace uPocketWS::Web
