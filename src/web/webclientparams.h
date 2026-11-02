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
        rpcLog = nullptr;
    }

    /**
     * @brief uploadDirs Canonical absolute directories (resolved from the
     *        --upload-dir CLI options, relative to the document root) that are
     *        the only locations where uploads are allowed (creating new files).
     *        Sub-directories are allowed. Empty -> uploads disabled.
     */
    std::vector<std::string> uploadDirs;
    /**
     * @brief overwriteDirs Directories (and sub-dirs) where uploading over an
     *        existing file is allowed (--overwrite-dir). Empty -> no overwrite.
     */
    std::vector<std::string> overwriteDirs;
    /**
     * @brief deleteDirs Directories (and sub-dirs) where file/directory
     *        deletion is allowed (--delete-dir). Empty -> no deletion.
     */
    std::vector<std::string> deleteDirs;
    /**
     * @brief mkdirDirs Directories (and sub-dirs) where new directories can
     *        be created (--mkdir-dir). Empty -> no mkdir.
     */
    std::vector<std::string> mkdirDirs;
    /**
     * @brief archiveDirs Directories (and sub-dirs) that can be downloaded as
     *        on-the-fly tar.gz (--archive-dir). Empty -> no archive.
     */
    std::vector<std::string> archiveDirs;
    /**
     * @brief executeDirs Directories (and sub-dirs) where executable files can
     *        be run CGI-like (--execute-dir). Empty -> no execution.
     */
    std::vector<std::string> executeDirs;
    /** @brief uploadNameRe Optional regex the uploaded filename must fully match. */
    std::optional<boost::regex> uploadNameRe;

    std::string softwareVersion;
    std::string httpDocumentRootDir;
    Mantids30::Program::Logs::RPCLog *rpcLog;
    std::string user, pass; // Basic auth credentials (pass empty -> auth disabled)
};
} // namespace uPocketWS::Web
