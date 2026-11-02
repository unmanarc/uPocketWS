#pragma once

// Minimal self-contained HTML5 error pages (light theme, no external resources).
#define HTML_ERROR_PAGE(CODE, TITLE)                                                                                                      \
    "<!DOCTYPE html>\n<html lang='en'><head><meta charset='utf-8'><meta name='viewport' content='width=device-width, initial-scale=1'>"   \
    "<title>" CODE " " TITLE "</title><style>"                                                                                             \
    "body{margin:0;min-height:100vh;display:grid;place-items:center;background:#f5f7fb;color:#1f2937;"                                     \
    "font:15px/1.5 system-ui,-apple-system,'Segoe UI',Roboto,Arial,sans-serif}"                                                            \
    "main{text-align:center;padding:40px 48px;background:#fff;border:1px solid #e3e7ef;border-radius:12px;"                                \
    "box-shadow:0 4px 16px rgba(16,24,40,.06)}"                                                                                            \
    "h1{margin:0;font-size:56px;line-height:1;color:#2563eb}p{margin:10px 0 0;color:#6b7280}a{color:#2563eb;text-decoration:none}"          \
    "</style></head><body><main><h1>" CODE "</h1><p>" TITLE "</p></main></body></html>\n"

static constexpr const char *HTML_404 = HTML_ERROR_PAGE("404", "Not Found");

static constexpr const char *HTML_401 = HTML_ERROR_PAGE("401", "Unauthorized");

// 503 HTTP full response (headers + body). CRLF required by HTTP protocol.
static constexpr const char *HTML_503_RESPONSE =
        "HTTP/1.1 503 Service Temporarily Unavailable\r\n"
        "Content-Type: text/html; charset=UTF-8\r\n"
        "Connection: close\r\n"
        "\r\n" HTML_ERROR_PAGE("503", "Service Temporarily Unavailable");

#undef HTML_ERROR_PAGE
