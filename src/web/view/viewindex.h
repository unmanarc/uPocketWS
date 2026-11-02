#pragma once

#include <Mantids30/Protocol_HTTP/httpv1_server.h>

#include <filesystem>
#include <string>

namespace uPocketWS::Web
{
    class WebClientHdlr;
}

namespace uPocketWS::Web::View
{
    /**
     * @brief IndexView Renders the HTML directory listing (header, rows, footer)
     *        into the response stream. Pure presentation: no request decisions.
     */
    class IndexView
    {
    public:
        static void render(WebClientHdlr &srv, const Mantids30::Network::Protocol::HTTP::HTTPv1_Server::LocalRequestedFileInfo &fileInfo);

    private:
        static std::string renderRow(const std::filesystem::directory_entry &entry, bool canModify);
    };
}
