#pragma once

#include <Mantids30/Protocol_HTTP/httpv1_server.h>

#include <string>

namespace uPocketWS::Web {
struct webClientParams;

/**
     * @brief WebRequestContext Mutable per-request state shared between the
     *        handler callbacks (onHTTPClientHeadersReceived /
     *        onHTTPClientContentReceived) and the MIME upload trampoline, which
     *        only receives a void* context.
     *
     * Deliberately kept out of webClientParams: that struct is immutable
     * server-wide configuration, while this one changes with every connection.
     */
struct WebRequestContext
{
    void reset()
    {
        fileInfo.reset();
        uploadDirBase.clear();
        remoteIP.clear();
        uploadOK = false;
    }

    const webClientParams *cfg = nullptr;                                               // Server configuration (needed by the MIME upload trampoline)
    Mantids30::Network::Protocol::HTTP::HTTPv1_Server::LocalRequestedFileInfo fileInfo; // Resolved once by the dispatcher

    std::string uploadDirBase; // Absolute path of the upload target directory (resolved from the request URI)
    std::string remoteIP;      // Remote client address (used by upload logs)
    bool uploadOK = false;     // Result flag written by the MIME part callback, read when the body is fully received
};
} // namespace uPocketWS::Web
