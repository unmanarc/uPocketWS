#include "web/view/viewindex.h"
#include "utils/webutils.h"
#include "web/view/html/html_index_css.h"
#include "web/view/html/html_index_templates.h"
#include "web/view/html/html_index_upload.h"
#include "web/webclienthdlr.h"
#include "web/handlers/webhandlers.h"

#include <Mantids30/Protocol_HTTP/streamencoder_url.h>

#include <algorithm>
#include <cctype>
#include <cstring>
#include <ctime>
#include <filesystem>
#include <sys/stat.h>
#include <vector>

using namespace Mantids30;
using HTTPServer = Mantids30::Network::Protocol::HTTP::HTTPv1_Server;
namespace fs = std::filesystem;

namespace uPocketWS::Web::View
{
    // snprintf into a std::string (templates are trusted, args are pre-encoded).
    template <typename... Args>
    static std::string format(const char *fmt, Args... args)
    {
        const int len = snprintf(nullptr, 0, fmt, args...);
        if (len <= 0)
            return "";
        std::string out(static_cast<size_t>(len) + 1, '\0');
        snprintf(out.data(), out.size(), fmt, args...);
        out.resize(static_cast<size_t>(len));
        return out;
    }

    static std::string lower(std::string s)
    {
        std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) { return std::tolower(c); });
        return s;
    }

    std::string IndexView::renderRow(const fs::directory_entry &entry, bool canModify)
    {
        const std::string name = entry.path().filename().string();
        const std::string encHtmlName = HTTPServer::htmlEncode(name);

        std::error_code sec;
        const fs::file_status st = entry.status(sec);
        if (sec)
            return "";
        const bool isDir = (st.type() == fs::file_type::directory);

        // uid/gid are not exposed by std::filesystem.
        struct stat64 rawStat;
        if (stat64(entry.path().c_str(), &rawStat) != 0)
            memset(&rawStat, 0, sizeof(rawStat));

        time_t t = rawStat.st_mtime;
        struct tm lt;
        localtime_r(&t, &lt);
        char timebufMtime[64], timebufIso[64];
        strftime(timebufMtime, sizeof(timebufMtime), "%Y-%m-%d %H:%M", &lt);
        strftime(timebufIso, sizeof(timebufIso), "%Y-%m-%dT%H:%M:%S%z", &lt);

        const fs::perms perms = st.permissions();
        const int statchmod = static_cast<int>(perms & fs::perms::all);
        const fs::perms others = perms & fs::perms::others_all;
        const bool worldWX = others == (fs::perms::others_write | fs::perms::others_exec) || others == (fs::perms::others_read | fs::perms::others_write | fs::perms::others_exec);
        const std::string chmodval = format(worldWX ? HTML_CHMOD_WARNING : HTML_CHMOD_NORMAL, statchmod);

        const std::string encName = Memory::Streams::Encoders::URL::encodeURLStr(name);

        std::string delCell;
        if (canModify)
        {
            // POSTed to the current directory: ./?action=delete&file=<name>
            const std::string delHref = "./?action=delete&file=" + encName;
            delCell = format(HTML_INDEX_DELETE_CELL, delHref.c_str(), encHtmlName.c_str(), isDir ? "1" : "0");
        }

        const int64_t sizeBytes = isDir ? 0 : static_cast<int64_t>(rawStat.st_size);
        const std::string fancySize = isDir ? "&mdash;" : Utils::getFancySize(static_cast<size_t>(rawStat.st_size));

        return format(HTML_INDEX_ROW, isDir ? "dir" : "file", encHtmlName.c_str(), sizeBytes, static_cast<int64_t>(rawStat.st_mtime), (encName + (isDir ? "/" : "")).c_str(),
                      isDir ? "folder" : "file", encHtmlName.c_str(), fancySize.c_str(), timebufIso, timebufMtime, static_cast<int32_t>(rawStat.st_uid),
                      static_cast<int32_t>(rawStat.st_gid), chmodval.c_str(), delCell.c_str());
    }

    void IndexView::render(WebClientHdlr &srv, const HTTPServer::LocalRequestedFileInfo &fileInfo)
    {
        const webClientParams &cfg = srv.params();
        auto out = srv.getResponseContentStreamableObject();

        srv.serverResponse.contentType = "text/html; charset=utf-8";

        const std::string encPath = HTTPServer::htmlEncode(fileInfo.relativePath);
        const bool canModify = cfg.overwrite;

        // Only show the upload dropzone if this directory is an allowed upload target
        // (an upload dir or a sub-dir of one). Shared with the upload handler so the
        // GUI and the backend stay in sync.
        bool canUpload = false;
        if (!cfg.uploadDirs.empty())
        {
            std::error_code checkEc;
            auto canonicalCurrent = fs::canonical(fileInfo.fullPath, checkEc);
            if (!checkEc)
                canUpload = Handlers::UploadHandler::isUploadDirAllowed(cfg, canonicalCurrent.string());
        }

        std::string toolbarActions;
        if (cfg.targz)
            toolbarActions += HTML_TARGZ_LINK;
        if (canModify)
            toolbarActions += HTML_MKDIR_FORM;

        out->strPrintf(HTML_INDEX_HEADER, encPath.c_str(), HTML_CSS_PAGE, HTML_ICON_SPRITE, encPath.c_str(), toolbarActions.c_str(), canUpload ? HTML_UPLOAD_FORM : "",
                       canModify ? HTML_INDEX_DELETE_HEADER_COL : "");

        std::error_code ec;
        if (!fs::is_directory(fileInfo.fullPath, ec) || ec)
        {
            out->writeString(HTML_INDEX_FOLDER_INACCESSIBLE);
        }
        else
        {
            if (fileInfo.relativePath != "/")
                out->strPrintf(HTML_INDEX_PARENT_ROW, canModify ? HTML_INDEX_EMPTY_ACT_CELL : "");

            std::vector<fs::directory_entry> entries;
            for (auto it = fs::directory_iterator(fileInfo.fullPath, ec); !ec && it != fs::directory_iterator(); it.increment(ec))
                entries.push_back(*it);

            // Directories first, then case-insensitive by name.
            std::sort(entries.begin(), entries.end(), [](const fs::directory_entry &a, const fs::directory_entry &b) {
                const bool aDir = a.is_directory(), bDir = b.is_directory();
                if (aDir != bDir)
                    return aDir;
                return lower(a.path().filename().string()) < lower(b.path().filename().string());
            });

            for (const auto &entry : entries)
                out->writeString(renderRow(entry, canModify));

            out->writeString(HTML_INDEX_TABLE_CLOSE);
        }

        std::string js = HTML_JS_COMMON;
        if (canUpload)
            js += HTML_JS_UPLOAD;
        if (canModify)
            js += std::string(HTML_JS_DELETE) + HTML_JS_MKDIR;

        out->strPrintf(HTML_INDEX_FOOTER, cfg.softwareVersion.c_str(), canModify ? HTML_DIALOGS : "", js.c_str());
    }
}
