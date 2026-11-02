#pragma once

#include <inttypes.h>

// NOTE: every template in this file that is passed to strPrintf/snprintf as a
// format string must escape literal '%' characters as '%%'.

// SVG icon sprite (Lucide/Feather style, inline: no external dependencies).
// Passed as a "%s" argument, so it is not a format string.
static constexpr const char *HTML_ICON_SPRITE = R"(<svg xmlns="http://www.w3.org/2000/svg" style="display:none" aria-hidden="true">
  <symbol id="i-server" viewBox="0 0 24 24"><rect x="2" y="2" width="20" height="8" rx="2"/><rect x="2" y="14" width="20" height="8" rx="2"/><line x1="6" y1="6" x2="6.01" y2="6"/><line x1="6" y1="18" x2="6.01" y2="18"/></symbol>
  <symbol id="i-folder" viewBox="0 0 24 24"><path d="M22 19a2 2 0 0 1-2 2H4a2 2 0 0 1-2-2V5a2 2 0 0 1 2-2h5l2 3h9a2 2 0 0 1 2 2z"/></symbol>
  <symbol id="i-up" viewBox="0 0 24 24"><polyline points="9 14 4 9 9 4"/><path d="M20 20v-7a4 4 0 0 0-4-4H4"/></symbol>
  <symbol id="i-file" viewBox="0 0 24 24"><path d="M13 2H6a2 2 0 0 0-2 2v16a2 2 0 0 0 2 2h12a2 2 0 0 0 2-2V9z"/><polyline points="13 2 13 9 20 9"/></symbol>
  <symbol id="i-text" viewBox="0 0 24 24"><path d="M14 2H6a2 2 0 0 0-2 2v16a2 2 0 0 0 2 2h12a2 2 0 0 0 2-2V8z"/><polyline points="14 2 14 8 20 8"/><line x1="16" y1="13" x2="8" y2="13"/><line x1="16" y1="17" x2="8" y2="17"/><line x1="10" y1="9" x2="8" y2="9"/></symbol>
  <symbol id="i-image" viewBox="0 0 24 24"><rect x="3" y="3" width="18" height="18" rx="2"/><circle cx="8.5" cy="8.5" r="1.5"/><polyline points="21 15 16 10 5 21"/></symbol>
  <symbol id="i-video" viewBox="0 0 24 24"><polygon points="23 7 16 12 23 17 23 7"/><rect x="1" y="5" width="15" height="14" rx="2"/></symbol>
  <symbol id="i-audio" viewBox="0 0 24 24"><path d="M9 18V5l12-2v13"/><circle cx="6" cy="18" r="3"/><circle cx="18" cy="16" r="3"/></symbol>
  <symbol id="i-archive" viewBox="0 0 24 24"><polyline points="21 8 21 21 3 21 3 8"/><rect x="1" y="3" width="22" height="5"/><line x1="10" y1="12" x2="14" y2="12"/></symbol>
  <symbol id="i-code" viewBox="0 0 24 24"><polyline points="16 18 22 12 16 6"/><polyline points="8 6 2 12 8 18"/></symbol>
  <symbol id="i-pdf" viewBox="0 0 24 24"><path d="M14 2H6a2 2 0 0 0-2 2v16a2 2 0 0 0 2 2h12a2 2 0 0 0 2-2V8z"/><polyline points="14 2 14 8 20 8"/><path d="M9 15h6"/><path d="M9 18h3"/></symbol>
  <symbol id="i-upload" viewBox="0 0 24 24"><polyline points="16 16 12 12 8 16"/><line x1="12" y1="12" x2="12" y2="21"/><path d="M20.39 18.39A5 5 0 0 0 18 9h-1.26A8 8 0 1 0 3 16.3"/></symbol>
  <symbol id="i-download" viewBox="0 0 24 24"><path d="M21 15v4a2 2 0 0 1-2 2H5a2 2 0 0 1-2-2v-4"/><polyline points="7 10 12 15 17 10"/><line x1="12" y1="15" x2="12" y2="3"/></symbol>
  <symbol id="i-folder-plus" viewBox="0 0 24 24"><path d="M22 19a2 2 0 0 1-2 2H4a2 2 0 0 1-2-2V5a2 2 0 0 1 2-2h5l2 3h9a2 2 0 0 1 2 2z"/><line x1="12" y1="11" x2="12" y2="17"/><line x1="9" y1="14" x2="15" y2="14"/></symbol>
  <symbol id="i-trash" viewBox="0 0 24 24"><polyline points="3 6 5 6 21 6"/><path d="M19 6l-1 14a2 2 0 0 1-2 2H8a2 2 0 0 1-2-2L5 6"/><path d="M10 11v6"/><path d="M14 11v6"/><path d="M9 6V4a1 1 0 0 1 1-1h4a1 1 0 0 1 1 1v2"/></symbol>
  <symbol id="i-search" viewBox="0 0 24 24"><circle cx="11" cy="11" r="8"/><line x1="21" y1="21" x2="16.65" y2="16.65"/></symbol>
</svg>)";

// Index page header. Used with strPrintf:
//   %s = path (title), %s = CSS block, %s = icon sprite, %s = path (breadcrumb fallback),
//   %s = toolbar actions (tar.gz / new dir buttons), %s = upload drop zone,
//   %s = delete column header (empty when delete not allowed)
static constexpr const char *HTML_INDEX_HEADER =
        "<!DOCTYPE html>\n"
        "<html lang='en'>\n"
        "<head>\n"
        "    <meta charset='utf-8'>\n"
        "    <meta name='viewport' content='width=device-width, initial-scale=1'>\n"
        "    <meta name='color-scheme' content='light'>\n"
        "    <title>Index of %s</title>\n"
        "    <link rel='icon' href=\"data:image/svg+xml,<svg xmlns='http://www.w3.org/2000/svg' viewBox='0 0 24 24' fill='none' stroke='rgb(37,99,235)' stroke-width='2' stroke-linecap='round' stroke-linejoin='round'><path d='M22 19a2 2 0 0 1-2 2H4a2 2 0 0 1-2-2V5a2 2 0 0 1 2-2h5l2 3h9a2 2 0 0 1 2 2z'/></svg>\">\n"
        "%s\n"
        "</head>\n"
        "<body>\n"
        "%s\n"
        "<header class='app-header'>\n"
        "    <div class='wrap'>\n"
        "        <div class='brand' aria-hidden='true'><svg class='ic'><use href='#i-server'/></svg></div>\n"
        "        <div class='title-block'>\n"
        "            <h1>Index of</h1>\n"
        "            <nav class='crumbs' aria-label='Breadcrumb'><ol id='crumbs'><li class='crumb-root'><a href='/'>/</a></li><li aria-current='page'>%s</li></ol></nav>\n"
        "        </div>\n"
        "    </div>\n"
        "</header>\n"
        "<main>\n"
        "    <div class='wrap'>\n"
        "        <div class='toolbar' role='toolbar'>\n"
        "            <label class='search'><span class='sr-only'>Filter</span><svg class='ic'><use href='#i-search'/></svg>"
        "<input type='search' id='filter' placeholder='Filter files...' autocomplete='off'></label>\n"
        "            <span class='spacer'></span>\n"
        "%s\n"
        "        </div>\n"
        "%s\n"
        "        <div class='card'>\n"
        "        <table class='listing' id='listing'>\n"
        "            <thead>\n"
        "                <tr>\n"
        "                    <th scope='col' class='c-name' data-sort='name'>Name</th>\n"
        "                    <th scope='col' class='c-size' data-sort='size'>Size</th>\n"
        "                    <th scope='col' class='c-mtime' data-sort='mtime'>Modified</th>\n"
        "                    <th scope='col' class='c-owner'>UID:GID</th>\n"
        "                    <th scope='col' class='c-perm'>Mode</th>\n"
        "%s"
        "                </tr>\n"
        "            </thead>\n"
        "            <tbody>\n";

// Parent directory ("..") row. Used with strPrintf: %s = extra empty cell when delete column exists
static constexpr const char *HTML_INDEX_PARENT_ROW =
        "                <tr class='t-up' data-up='1'>\n"
        "                    <td class='c-name'><a href='../'><svg class='ic'><use href='#i-up'/></svg><span>Parent directory</span></a></td>\n"
        "                    <td class='c-size'></td><td class='c-mtime'></td><td class='c-owner'></td><td class='c-perm'></td>%s\n"
        "                </tr>\n";

// Table row for one file/directory entry. Used with strPrintf:
//   %s = type class ("dir"/"file"), %s = HTML-encoded name (data-name), %" PRIi64 " = size in bytes,
//   %" PRIi64 " = mtime (epoch), %s = URL-encoded href, %s = icon id ("folder"/"file"),
//   %s = HTML-encoded name, %s = human size, %s = ISO datetime, %s = display datetime,
//   %" PRIi32 ":%" PRIi32 " = UID:GID, %s = permissions, %s = delete cell (empty when delete not allowed)
static constexpr const char *HTML_INDEX_ROW =
        "                <tr class='t-%s' data-name='%s' data-size='%" PRIi64 "' data-mtime='%" PRIi64 "'>\n"
        "                    <td class='c-name'><a href='%s'><svg class='ic'><use href='#i-%s'/></svg><span>%s</span></a></td>\n"
        "                    <td class='c-size'>%s</td>\n"
        "                    <td class='c-mtime'><time datetime='%s'>%s</time></td>\n"
        "                    <td class='c-owner'>%" PRIi32 ":%" PRIi32 "</td>\n"
        "                    <td class='c-perm'>%s</td>\n"
        "%s"
        "                </tr>\n";

// Delete column header cell (only present when can-overwrite/delete is allowed)
static constexpr const char *HTML_INDEX_DELETE_HEADER_COL =
        "                    <th scope='col' class='c-act'><span class='sr-only'>Actions</span></th>\n";

// Empty cell used in the parent row when the delete column exists
static constexpr const char *HTML_INDEX_EMPTY_ACT_CELL = "<td class='c-act'></td>";

// Per-row delete cell. Used with snprintf:
//   %s = URL-encoded delete href, %s = HTML-encoded name, %s = "1" for directory / "0" for file
// No inline JS: the click is handled by a delegated listener (HTML_JS_DELETE).
static constexpr const char *HTML_INDEX_DELETE_CELL =
        "                    <td class='c-act'><button type='button' class='btn-icon btn-del' data-href='%s' data-name='%s' data-dir='%s' "
        "title='Delete' aria-label='Delete'><svg class='ic'><use href='#i-trash'/></svg></button></td>\n";

static constexpr const char *HTML_INDEX_TABLE_CLOSE =
        "            </tbody>\n"
        "        </table>\n"
        "        </div>\n";

// Folder not accessible error
static constexpr const char *HTML_INDEX_FOLDER_INACCESSIBLE =
        "            </tbody>\n"
        "        </table>\n"
        "        </div>\n"
        "        <p class='alert' role='alert'>Folder not accessible</p>\n";

// Index page footer. Used with strPrintf:
//   %s = software version, %s = dialogs markup, %s = JS block
static constexpr const char *HTML_INDEX_FOOTER =
        "    </div>\n"
        "</main>\n"
        "<footer class='app-footer'>\n"
        "    <div class='wrap'>Provided by <a href='https://github.com/unmanarc/uPocketWS'>Unmanarc's Pocket Web Server v%s</a></div>\n"
        "</footer>\n"
        "<div class='toasts' id='toasts' aria-live='polite'></div>\n"
        "%s\n"
        "%s\n"
        "</body>\n"
        "</html>\n";

// Inline HTML snippets used with snprintf for permission display
static constexpr const char *HTML_CHMOD_WARNING = "<span class='perm-warn' title='Others can write and execute'>%o</span>";
static constexpr const char *HTML_CHMOD_NORMAL = "%o";
