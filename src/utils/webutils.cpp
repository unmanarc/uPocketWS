#include "utils/webutils.h"

#include <boost/algorithm/string/case_conv.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/algorithm/string/regex.hpp>
#include <boost/regex.hpp>

#include <cstdio>
#include <filesystem>

namespace uPocketWS::Utils {
std::string getFancySize(size_t value)
{
    const char *szNames[] = {"B", "kB", "MB", "GB", "TB"};

    size_t pos = 0, remainder = 0;

    for (; value >= 1024 && pos < (sizeof szNames / sizeof *szNames); value /= 1024)
    {
        remainder = (value % 1024);
        pos++;
    }

    char output[64];

    if (remainder)
        sprintf(output, "%.1f %s", (double) value + ((double) remainder / (double) 1024), szNames[pos]);
    else
        sprintf(output, "%zu %s", value, szNames[pos]);
    return std::string(output);
}

std::string sanitizeDownloadFileName(const std::string &name)
{
    static const boost::regex unsafeCharsRe("[^a-zA-Z0-9._\\-]");
    std::string safe = name;
    boost::replace_all_regex(safe, unsafeCharsRe, std::string("_"));
    return safe;
}

bool containOnlyAllowedChars(const std::string &str)
{
    static const boost::regex allowedCharsRe("[a-zA-Z0-9.,!@_\\-+=/:]*");
    return boost::regex_match(str, allowedCharsRe);
}

bool isPlainFileName(const std::string &name)
{
    if (name.empty() || name == "." || name == "..")
        return false;
    return name.find('/') == std::string::npos && name.find('\\') == std::string::npos;
}

bool isRegularFile(const std::string &path)
{
    std::error_code ec;
    auto status = std::filesystem::status(path, ec);
    return !ec && status.type() == std::filesystem::file_type::regular;
}

std::string stripSuffix(const std::string &str, const std::string &suffix)
{
    if (suffix.empty() || str.size() < suffix.size())
        return str;
    if (str.compare(str.size() - suffix.size(), suffix.size(), suffix) != 0)
        return str;
    return str.substr(0, str.size() - suffix.size());
}
} // namespace uPocketWS::Utils
