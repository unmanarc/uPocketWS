#pragma once

#include <Mantids30/Program_Logs/rpclog.h>

#include <string>

namespace uPocketWS::Web {
/**
     * @brief webClientParams Immutable per-server configuration handed to every
     *        WebClientHdlr connection. Per-request mutable state (upload dir,
     *        remote IP, upload result) lives in WebRequestContext instead.
     */
struct webClientParams
{
    webClientParams()
    {
        execute = false;
        rpcLog = nullptr;
        targz = false;
        uploads = false;
        overwrite = false;
    }

    bool execute;   // Allow CGI-like execution of marked executables
    bool targz;     // Allow on-the-fly tar.gz of directories
    bool uploads;   // Allow multipart file uploads
    bool overwrite; // Allow overwrite on upload, delete, and mkdir
    std::string softwareVersion;
    std::string httpDocumentRootDir;
    Mantids30::Program::Logs::RPCLog *rpcLog;
    std::string user, pass; // Basic auth credentials (pass empty -> auth disabled)
};
} // namespace uPocketWS::Web
