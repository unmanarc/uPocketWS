#pragma once

#include <Mantids30/Program_Logs/rpclog.h>

#include <boost/regex.hpp>

#include <optional>
#include <string>
#include <vector>

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
        overwrite = false;
    }

    bool execute;   // Allow CGI-like execution of marked executables
    bool targz;     // Allow on-the-fly tar.gz of directories
    bool overwrite; // Allow overwrite on upload, delete, and mkdir

    /**
     * @brief uploadDirs Canonical absolute directories (resolved from the
     *        --upload-dir CLI options, relative to the document root) that are
     *        the only locations where uploads are allowed. Empty -> uploads
     *        disabled.
     */
    std::vector<std::string> uploadDirs;
    /** @brief uploadNameRe Optional regex the uploaded filename must fully match. */
    std::optional<boost::regex> uploadNameRe;

    std::string softwareVersion;
    std::string httpDocumentRootDir;
    Mantids30::Program::Logs::RPCLog *rpcLog;
    std::string user, pass; // Basic auth credentials (pass empty -> auth disabled)
};
} // namespace uPocketWS::Web
