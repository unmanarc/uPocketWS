#include "web/webclienthdlr.h"
#include "web/handlers/webhandlers.h"
#include "web/view/html/html_errors.h"
#include "web/weblog.h"

#include <Mantids30/Helpers/random.h>
#include <Mantids30/Program_Logs/applog.h>

using namespace Mantids30;
using namespace Mantids30::Network::Protocol;
using namespace Mantids30::Program;

namespace uPocketWS::Web {
WebClientHdlr::WebClientHdlr(const std::shared_ptr<Memory::Streams::StreamableObject> &sock)
    : HTTPv1_Server(sock)
{
    m_ctx.cfg = &m_params;
}

void WebClientHdlr::setWebClientParameters(const webClientParams &newWebClientParameters)
{
    m_params = newWebClientParameters;
}

WebClientHdlr::Code WebClientHdlr::respond(Code ret, const std::string &uri)
{
    Log::logRpc(m_params.rpcLog,
                ret == HTTP::Status::Code::S_200_OK ? Logs::INFO : Logs::WARNING,
                remoteIP(),
                Log::SHORT_SIZE,
                "R/%03d:%s: %s",
                static_cast<int>(ret),
                clientRequest.requestLine.getHTTPMethod().c_str(),
                uri.c_str());
    return ret;
}

std::string WebClientHdlr::getVar(const std::string &name)
{
    auto vars = clientRequest.getVarsBySource(HTTP::Source::GET);
    return (vars && vars->exist(name)) ? vars->getStringValue(name) : "";
}

void WebClientHdlr::writeBody(const std::string &str)
{
    getResponseContentStreamableObject()->writeString(str);
}

// All POST requests must carry the anti-CSRF token (header == cookie).
static bool checkCSRFTokenOnPOST(WebClientHdlr &srv)
{
    if (srv.clientRequest.requestLine.getHTTPMethod() != "POST")
        return true;
    // An empty token must never match: a cross-site request carries neither the
    // SameSite=Strict cookie nor the header, and "" == "" would accept it.
    const std::string cookieToken = srv.clientRequest.getCookie("CSRFToken");
    if (!cookieToken.empty() && srv.clientRequest.headers.getOptionRawStringByName("X-CSRFToken") == cookieToken)
        return true;

    Log::logRpc(srv.params().rpcLog, Logs::ERROR, srv.remoteIP(), Log::FULL_SIZE, "R/POST-FAIL-CSRF: %s", srv.clientRequest.getURI().c_str());
    return false;
}

static void createCSRFTokenCookie(WebClientHdlr &srv)
{
    if (srv.clientRequest.requestLine.getHTTPMethod() != "GET")
        return;

    HTTP::Headers::Cookie val;
    // Mantids30 seeds the CSPRNG once internally (RAND_bytes); the call is
    // thread-safe and cheap enough per request.
    val.value = Mantids30::Helpers::Random::createRandomString(32);
    val.secure = srv.clientRequest.networkClientInfo.isSecure; // avoid MITM on the CSRF token over HTTPS
    val.httpOnly = false;                                      // must be readable by the JS
    val.path = "/";
    val.setAsSessionCookie();
    val.sameSitePolicy = HTTP::Headers::Cookie::SameSitePolicy::STRICT;
    srv.serverResponse.setCookie("CSRFToken", val);
}

bool WebClientHdlr::onHTTPClientHeadersReceived()
{
    m_ctx.reset();
    m_ctx.remoteIP = remoteIP();

    // Reset the MIME callback (the object may be reused by keep-alive requests).
    if (clientRequest.content.getContainerType() == HTTP::Content::ContainerType::MIME)
        clientRequest.content.getMultiPartVars()->setCallbackOnHeaderReady(MIME::MIME_Message::MIMECallback());

    if (!checkCSRFTokenOnPOST(*this))
        return true; // rejected later in onHTTPClientContentReceived

    Handlers::UploadHandler upload;
    if (upload.canHandle(*this, m_ctx))
        return Handlers::UploadHandler::prepareFromClientHeader(*this, m_ctx);

    return true;
}

WebClientHdlr::Code WebClientHdlr::onHTTPClientContentReceived()
{
    if (!checkCSRFTokenOnPOST(*this))
        return HTTP::Status::Code::S_401_UNAUTHORIZED;

    if (!m_params.pass.empty())
    {
        if (!clientRequest.basicAuth.isEnabled || clientRequest.basicAuth.password != m_params.pass || clientRequest.basicAuth.username != m_params.user)
        {
            writeBody(HTML_401);
            serverResponse.sWWWAuthenticateRealm = "Authentication Required";
            return respond(HTTP::Status::Code::S_401_UNAUTHORIZED, clientRequest.getURI());
        }
    }

    createCSRFTokenCookie(*this);

    return m_dispatcher.dispatch(*this, m_ctx);
}
} // namespace uPocketWS::Web
