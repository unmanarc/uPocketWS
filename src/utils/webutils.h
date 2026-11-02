#pragma once

#include <string>

namespace uPocketWS::Utils
{
    /**
     * @brief getFancySize Human readable size (B, kB, MB, GB, TB).
     */
    std::string getFancySize(size_t value);

    /**
     * @brief sanitizeDownloadFileName Replace every char that is not [a-zA-Z0-9._-] with '_'
     * @return The sanitized name (safe for Content-Disposition: filename).
     */
    std::string sanitizeDownloadFileName(const std::string &name);

    /**
     * @brief containOnlyAllowedChars Base64-like safe charset for CGI env vars
     * @return true if the string matches [a-zA-Z0-9.,!@_\-+=/:]*
     */
    bool containOnlyAllowedChars(const std::string &str);

    /**
     * @brief isPlainFileName Validate an entry name that will be resolved
     * inside a known parent directory (no path separators, not "."/"..").
     * @return true if the name is safe to concatenate to a directory.
     */
    bool isPlainFileName(const std::string &name);

    /**
     * @brief isProtectedMarkerFile Check if the resolved path points to one of
     * the internal marker files (.dontupload/.uploadname/.upload).
     * Centralizes the list that was duplicated across the upload and delete
     * code paths.
     * @param fullPath resolved absolute path to check
     */
    bool isProtectedMarkerFile(const std::string &fullPath);

    /**
     * @brief isRegularFile Check if the path exists and is a regular file
     * (a directory or missing path returns false; a symlink to a regular
     * file counts).
     * @param path path to check
     */
    bool isRegularFile(const std::string &path);

    /**
     * @brief stripSuffix Remove a known suffix from the end of a string.
     * @return the string without the suffix (unchanged if it does not end with it).
     */
    std::string stripSuffix(const std::string &str, const std::string &suffix);
}
