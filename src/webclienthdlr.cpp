#include "webclienthdlr.h"
#include "html_errors.h"
#include "html_index_css.h"
#include "html_index_templates.h"
#include "html_index_upload.h"
#include <Mantids30/Helpers/appexec.h>
#include <Mantids30/Helpers/random.h>
#include <Mantids30/Memory/b_mmap.h>
#include <Mantids30/Memory/streamable_process.h>
#include <Mantids30/Program_Logs/loglevels.h>
#include <Mantids30/Protocol_HTTP/rsp_status.h>
#include <Mantids30/Protocol_HTTP/streamencoder_url.h>
#include <boost/algorithm/string/case_conv.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/algorithm/string/regex.hpp>
#include <boost/range/end.hpp>
#include <boost/regex.hpp>

#include <Mantids30/Memory/streamable_file.h>

#include <algorithm>
#include <cctype>
#include <fcntl.h>
#include <filesystem>
#include <inttypes.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include <fstream>
#include <string>
#include <vector>

using namespace Mantids30;
using namespace Mantids30::Network::Protocol;
using namespace Mantids30::Network::Protocol::HTTP;
using namespace Mantids30::Network::Protocol::MIME;
using namespace Mantids30::Memory::Streams;
using namespace Mantids30::Program;
namespace fs = std::filesystem;

// TODO: HTML for uploads
// TODO: null stream por default

WebClientHdlr::WebClientHdlr(const std::shared_ptr<StreamableObject> &sock)
    : HTTPv1_Server(sock)
{}

WebClientHdlr::~WebClientHdlr() {}

bool WebClientHdlr::checkCSRFTokenOnPOST()
{
    if (clientRequest.requestLine.getHTTPMethod() == "POST")
    {
        // All POST should be checked with AntiCSRF token

        if (clientRequest.headers.getOptionRawStringByName("X-CSRFToken") != clientRequest.getCookie("CSRFToken"))
        {
            // All POST requests should come with anti-csrf token...
            webClientParameters.rpcLog->log(Logs::ERROR, clientRequest.networkClientInfo.REMOTE_ADDR, "", "", "", "fileServer", 65535, "R/POST-FAIL-CSRF: %s", clientRequest.getURI().c_str());
            // Ret false
            return false;
        }
    }
    // TOKEN OK.
    return true;
}

void cbFileTransferMIMEHeaderReady(void *obj, const std::string &partName, const std::shared_ptr<MIME::MIME_PartMessage> &partMessage)
{
    webClientParams *webClientParameters = (webClientParams *) obj;
    auto fileName = partMessage->getHeader()->getOptionByName("Content-Disposition")->getSubVar("filename");
    HTTPv1_Server::LocalRequestedFileInfo reqInfo;

    if (fs::exists(webClientParameters->uploadDirBase + "/.dontupload"))
    {
        webClientParameters->rpcLog->log(Logs::ERROR, webClientParameters->remoteIP, "", "", "", "fileServer", 65535, "R/UPLOAD-FAIL-PERMS-DIR: %s", webClientParameters->uploadDirBase.c_str());
        return;
    }
    if (fs::exists(webClientParameters->uploadDirBase + "/.uploadname"))
    {
        // Access new upload name:
        std::ifstream file;
        std::string line;

        file.open((webClientParameters->uploadDirBase + "/.uploadname").c_str());
        if (file)
        {
            if (getline(file, line))
            {
                while (line.back() == '\n' || line.back() == '\r')
                    line.pop_back();
                fileName = line;
            }
            file.close();
        }
        else
            return;
    }

    // Check if file exist:
    if (WebClientHdlr::resolveLocalFilePathFromURI0E("/" + fileName, webClientParameters->uploadDirBase, &reqInfo))
    {
        if (boost::ends_with(boost::to_lower_copy(reqInfo.fullPath), "/.dontupload") || boost::ends_with(boost::to_lower_copy(reqInfo.fullPath), "/.uploadname")
            || boost::ends_with(boost::to_lower_copy(reqInfo.fullPath), "/.upload"))
        {
            webClientParameters->rpcLog->log(Logs::CRITICAL, webClientParameters->remoteIP, "", "", "", "fileServer", 65535, "R/UPLOAD-NICETRY: %s", reqInfo.relativePath.c_str());
            return;
        }

        if (webClientParameters->overwrite)
        {
            // Allowed to overwrite...
            // if exist, and there is no transversal attack... overwrite...
            auto file = std::make_shared<StreamableFile>();
            if (file->open(reqInfo.fullPath.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0600) >= 0)
            {
                partMessage->getContent()->replaceContentContainer(file);
                webClientParameters->rpcLog->log(Logs::INFO, webClientParameters->remoteIP, "", "", "", "fileServer", 65535, "R/UPLOAD-OVERW-OK: %s", reqInfo.relativePath.c_str());
                webClientParameters->uploadOK = true;
            }
            else
            {
                webClientParameters->rpcLog->log(Logs::ERROR, webClientParameters->remoteIP, "", "", "", "fileServer", 65535, "R/UPLOAD-FAIL-PERMS1: %s", reqInfo.relativePath.c_str());
                file.reset();
            }
        }
        else
        {
            webClientParameters->rpcLog->log(Logs::ERROR, webClientParameters->remoteIP, "", "", "", "fileServer", 65535, "R/UPLOAD-FAIL-OVERWRITE: %s", reqInfo.relativePath.c_str());
        }
    }
    // TODO: URL Encoded file name (and filename field name)
    else if (WebClientHdlr::resolveLocalFilePathFromURI0NE("/" + fileName, webClientParameters->uploadDirBase, &reqInfo) && !reqInfo.isTransversal)
    {
        if (boost::ends_with(boost::to_lower_copy(reqInfo.fullPath), "/.dontupload") || boost::ends_with(boost::to_lower_copy(reqInfo.fullPath), "/.uploadname"))
        {
            webClientParameters->rpcLog->log(Logs::CRITICAL, webClientParameters->remoteIP, "", "", "", "fileServer", 65535, "R/UPLOAD-NICETRY: %s", reqInfo.relativePath.c_str());
            return;
        }

        // Yes or Not allowed to overwrite and file does not exist...
        auto file = std::make_shared<StreamableFile>();
        if (file->open(reqInfo.fullPath.c_str(), O_WRONLY | O_CREAT, 0600) >= 0)
        {
            partMessage->getContent()->replaceContentContainer(file);
            webClientParameters->rpcLog->log(Logs::INFO, webClientParameters->remoteIP, "", "", "", "fileServer", 65535, "R/UPLOAD-CREATE-OK: %s", reqInfo.relativePath.c_str());
            webClientParameters->uploadOK = true;
        }
        else
        {
            webClientParameters->rpcLog->log(Logs::ERROR, webClientParameters->remoteIP, "", "", "", "fileServer", 65535, "R/UPLOAD-FAIL-PERMS2: %s", reqInfo.relativePath.c_str());
            file.reset();
        }
    }
    else if (reqInfo.isTransversal)
    {
        webClientParameters->rpcLog->log(Logs::CRITICAL, webClientParameters->remoteIP, "", "", "", "fileServer", 65535, "R/UPLOAD-TRANSVERSAL2: %s", fileName.c_str());
    }
    else
    {
        webClientParameters->rpcLog->log(Logs::ERROR, webClientParameters->remoteIP, "", "", "", "fileServer", 65535, "R/UPLOAD-FAIL: %s", fileName.c_str());
    }
}

bool WebClientHdlr::onHTTPClientHeadersReceived()
{
    // Reset the callback.
    if (clientRequest.content.getContainerType() == Content::ContainerType::MIME)
        clientRequest.content.getMultiPartVars()->setCallbackOnHeaderReady(MIME::MIME_Message::MIMECallback());

    if (!checkCSRFTokenOnPOST())
        return true;

    if (clientRequest.requestLine.getHTTPMethod() == "POST")
    {
        // All POST should be checked with AntiCSRF token
        if (boost::ends_with(clientRequest.requestLine.getURI(), "/.upload") && webClientParameters.uploads == true)
        {
            // Reset:

            HTTPv1_Server::LocalRequestedFileInfo reqInfo;

            std::string uri = clientRequest.requestLine.getURI();

            if (HTTPv1_Server::resolveLocalFilePathFromURI0E(uri, webClientParameters.httpDocumentRootDir, &reqInfo))
            {
                // not allowed to upload here.
                webClientParameters.rpcLog
                    ->log(Logs::ERROR, clientRequest.networkClientInfo.REMOTE_ADDR, "", "", "", "fileServer", 65535, "R/UPLOAD-FAIL-NOTALLOWED: %s", clientRequest.getURI().c_str());
                return false;
            }

            uri.pop_back(); //u
            uri.pop_back(); //p
            uri.pop_back(); //l
            uri.pop_back(); //o
            uri.pop_back(); //a
            uri.pop_back(); //d

            webClientParameters.remoteIP = clientRequest.networkClientInfo.REMOTE_ADDR;

            if (HTTPv1_Server::resolveLocalFilePathFromURI0E(uri, webClientParameters.httpDocumentRootDir, &reqInfo))
            {
                if (reqInfo.isDirectory)
                {
                    webClientParameters.uploadDirBase = reqInfo.fullPath;
                    clientRequest.content.getMultiPartVars()->setCallbackOnHeaderReady(MIME::MIME_Message::MIMECallback(&cbFileTransferMIMEHeaderReady, &webClientParameters));
                    webClientParameters.rpcLog->log(Logs::INFO, clientRequest.networkClientInfo.REMOTE_ADDR, "", "", "", "fileServer", 65535, "R/UPLOAD-REQ: %s", clientRequest.getURI().c_str());
                }
            }
            else
            {
                webClientParameters.rpcLog->log(Logs::CRITICAL, webClientParameters.remoteIP, "", "", "", "fileServer", 65535, "R/UPLOAD-TRANSVERSAL1: %s", uri.c_str());
                return false;
            }
        }
    }

    return true;
}

Mantids30::Network::Protocol::HTTP::Status::Code WebClientHdlr::respond(Mantids30::Network::Protocol::HTTP::Status::Code ret, const std::string &uri)
{
    webClientParameters.rpcLog->log(ret == HTTP::Status::Code::S_200_OK ? Logs::INFO : Logs::WARNING,
                                    clientRequest.networkClientInfo.REMOTE_ADDR,
                                    "",
                                    "",
                                    "",
                                    "fileServer",
                                    2048,
                                    "R/%03d:%s: %s",
                                    static_cast<int>(ret),
                                    clientRequest.requestLine.getHTTPMethod().c_str(),
                                    uri.c_str());
    return ret;
}

void WebClientHdlr::createCSRFTokenCookie()
{
    if (clientRequest.requestLine.getHTTPMethod() == "GET")
    {
        Headers::Cookie val;
        val.value = Helpers::Random::createRandomString(32);

        // Secure if the communication is HTTPS... (important to avoid various mitm attacks to the CSRF)
        val.secure = clientRequest.networkClientInfo.isSecure;

        // Readable by the javascript:
        val.httpOnly = false;
        val.path = "/";
        val.setAsSessionCookie();
        val.sameSitePolicy = Headers::Cookie::SameSitePolicy::STRICT;
        serverResponse.setCookie("CSRFToken", val);
    }
}

Status::Code WebClientHdlr::onHTTPClientContentReceived()
{
    LocalRequestedFileInfo fileInfo;
    HTTP::Status::Code ret = HTTP::Status::Code::S_404_NOT_FOUND;

    if (!checkCSRFTokenOnPOST())
        return HTTP::Status::Code::S_401_UNAUTHORIZED;

    if (!webClientParameters.pass.empty())
    {
        if (!clientRequest.basicAuth.isEnabled || clientRequest.basicAuth.password != webClientParameters.pass || clientRequest.basicAuth.username != webClientParameters.user)
        {
            getResponseContentStreamableObject()->writeString(HTML_401);
            serverResponse.sWWWAuthenticateRealm = "Authentication Required";
            return respond(HTTP::Status::Code::S_401_UNAUTHORIZED, clientRequest.getURI().c_str());
        }
    }

    // Create CSRF Token on the cookie
    createCSRFTokenCookie();

    if (clientRequest.content.getContainerType() == Content::ContainerType::MIME)
    {
        if (clientRequest.requestLine.getHTTPMethod() == "POST" && boost::ends_with(clientRequest.requestLine.getURI(), "/.upload") && webClientParameters.uploads == true)
        {
            std::string uri = clientRequest.requestLine.getURI();
            uri.pop_back(); //u
            uri.pop_back(); //p
            uri.pop_back(); //l
            uri.pop_back(); //o
            uri.pop_back(); //a
            uri.pop_back(); //d
            getResponseContentStreamableObject()->writeString(webClientParameters.uploadOK ? "true" : "false");
            return respond(HTTP::Status::Code::S_200_OK, clientRequest.getURI().c_str());
        }
    }

    // Delete file/dir: POST /.delete?e=<urlencoded-entry-name>  (permission: can-overwrite, CSRF enforced above)
    // The entry name is passed as a query param because /.delete is POSTed to the directory that
    // contains the entry (same convention as .upload/.dontupload marker files).
    if (clientRequest.requestLine.getHTTPMethod() == "POST" && webClientParameters.overwrite == true && boost::ends_with(clientRequest.requestLine.getURI(), "/.delete"))
    {
        std::string uri = clientRequest.requestLine.getURI();
        for (int i = 0; i < 7; i++)
            uri.pop_back(); // remove "/.delete"

        // The entry name is passed as a GET query var ("e=<urlencoded name>").
        // libMantids already parses and URL-decodes GET vars into urlVars().
        auto getVars = clientRequest.getVarsBySource(HTTP::Source::GET);
        std::string entryName = getVars && getVars->exist("e") ? getVars->getStringValue("e") : "";

        if (entryName.empty())
        {
            getResponseContentStreamableObject()->writeString("false");
            return respond(HTTP::Status::Code::S_400_BAD_REQUEST, clientRequest.getURI().c_str());
        }

        // Only plain filenames are allowed (no path separators) to prevent escape.
        if (entryName.find('/') != std::string::npos || entryName.find('\\') != std::string::npos || entryName == "." || entryName == "..")
        {
            webClientParameters.rpcLog->log(Logs::CRITICAL, clientRequest.networkClientInfo.REMOTE_ADDR, "", "", "", "fileServer", 65535, "R/DELETE-BADNAME: %s", clientRequest.getURI().c_str());
            getResponseContentStreamableObject()->writeString("false");
            return respond(HTTP::Status::Code::S_403_FORBIDDEN, clientRequest.getURI().c_str());
        }

        LocalRequestedFileInfo delInfo;
        if (HTTPv1_Server::resolveLocalFilePathFromURI0E(uri, webClientParameters.httpDocumentRootDir, &delInfo))
        {
            // The parent directory (uri) must exist and be a directory (guaranteed by 0E).
            if (!delInfo.isDirectory)
            {
                getResponseContentStreamableObject()->writeString("false");
                return respond(HTTP::Status::Code::S_404_NOT_FOUND, clientRequest.getURI().c_str());
            }
            // Resolve the full path of the entry inside the directory.
            std::string entryFull = delInfo.fullPath + entryName;
            std::error_code ec;
            if (fs::exists(entryFull, ec) && !ec)
            {
                LocalRequestedFileInfo entryInfo;
                entryInfo.fullPath = entryFull;
                entryInfo.relativePath = delInfo.relativePath + entryName;
                entryInfo.isDirectory = fs::is_directory(entryFull);
                return handleDeleteRequest(entryInfo);
            }
            // Entry not found.
            getResponseContentStreamableObject()->writeString("false");
            return respond(HTTP::Status::Code::S_404_NOT_FOUND, clientRequest.getURI().c_str());
        }

        if (delInfo.isTransversal)
        {
            webClientParameters.rpcLog->log(Logs::CRITICAL, clientRequest.networkClientInfo.REMOTE_ADDR, "", "", "", "fileServer", 65535, "R/DELETE-TRANSVERSAL: %s", clientRequest.getURI().c_str());
            getResponseContentStreamableObject()->writeString("false");
            return respond(HTTP::Status::Code::S_403_FORBIDDEN, clientRequest.getURI().c_str());
        }

        // Parent directory not found under the document root.
        getResponseContentStreamableObject()->writeString("false");
        return respond(HTTP::Status::Code::S_404_NOT_FOUND, clientRequest.getURI().c_str());
    }

    // Create directory: POST /.mkdir?e=<urlencoded-entry-name>  (permission: can-overwrite, CSRF enforced above)
    // Same convention as /.delete: the entry name is a GET query param and the request is POSTed to
    // the directory that will contain the new directory.
    if (clientRequest.requestLine.getHTTPMethod() == "POST" && webClientParameters.overwrite == true && boost::ends_with(clientRequest.requestLine.getURI(), "/.mkdir"))
    {
        // Strip the "/.mkdir" suffix (7 chars) to obtain the parent directory URI.
        const std::string fullURI = clientRequest.requestLine.getURI();
        const std::string uri = fullURI.substr(0, fullURI.size() - 7);

        auto getVars = clientRequest.getVarsBySource(HTTP::Source::GET);
        std::string dirName = getVars && getVars->exist("e") ? getVars->getStringValue("e") : "";

        if (dirName.empty())
        {
            getResponseContentStreamableObject()->writeString("false");
            return respond(HTTP::Status::Code::S_400_BAD_REQUEST, clientRequest.getURI().c_str());
        }

        // Resolve the parent directory's absolute path.
        // Use 0E (existent file) since the parent directory must exist.
        // 0E does not append trailing slash for dirs — add it manually.
        std::string parentFull;
        bool parentIsDir = false;
        std::error_code ec;

        // The URI for the parent: if uri is empty (root), use "/" to resolve the root.
        const std::string parentUri = uri.empty() ? "/" : uri;

        LocalRequestedFileInfo parentInfo;
        if (HTTPv1_Server::resolveLocalFilePathFromURI0E(parentUri, webClientParameters.httpDocumentRootDir, &parentInfo) && !parentInfo.isTransversal)
        {
            parentFull = parentInfo.fullPath;
            parentIsDir = parentInfo.isDirectory;
            // 0E does not append trailing slash for dirs — add it here.
            if (!parentFull.empty() && parentFull.back() != '/')
                parentFull += '/';
        }

        if (!parentIsDir)
        {
            // Parent directory not found / not a directory under the document root.
            getResponseContentStreamableObject()->writeString("false");
            return respond(HTTP::Status::Code::S_404_NOT_FOUND, clientRequest.getURI().c_str());
        }

        // Validate the new directory name with the 0NE resolver (non-existent path variant),
        // resolving against the parent directory (which ends with "/"):
        //  - returns true  -> path does not exist and is inside the parent (fullPath is canonical, ready for create_directory)
        //  - returns false + isTransversal -> path escapes the parent directory (traversal attempt)
        //  - returns false otherwise -> path already exists (file or dir)
        // This covers every escape vector (e.g. %2e%2e%2f double-decode) without hand-checking characters.
        LocalRequestedFileInfo newDirInfo;
        if (!HTTPv1_Server::resolveLocalFilePathFromURI0NE("/" + dirName, parentFull, &newDirInfo))
        {
            if (newDirInfo.isTransversal)
            {
                webClientParameters.rpcLog->log(Logs::CRITICAL, clientRequest.networkClientInfo.REMOTE_ADDR, "", "", "", "fileServer", 65535, "R/MKDIR-TRANSVERSAL: %s", clientRequest.getURI().c_str());
                getResponseContentStreamableObject()->writeString("false");
                return respond(HTTP::Status::Code::S_403_FORBIDDEN, clientRequest.getURI().c_str());
            }
            // Refuse to overwrite an existing path (file or directory).
            getResponseContentStreamableObject()->writeString("false");
            return respond(HTTP::Status::Code::S_409_CONFLICT, clientRequest.getURI().c_str());
        }

        const std::string &newDirFull = newDirInfo.fullPath;

        if (fs::create_directory(newDirFull, ec))
        {
            webClientParameters.rpcLog->log(Logs::INFO, clientRequest.networkClientInfo.REMOTE_ADDR, "", "", "", "fileServer", 65535, "R/MKDIR-OK: %s", newDirFull.c_str());
            getResponseContentStreamableObject()->writeString("true");
            return respond(HTTP::Status::Code::S_200_OK, clientRequest.getURI().c_str());
        }

        webClientParameters.rpcLog->log(Logs::ERROR, clientRequest.networkClientInfo.REMOTE_ADDR, "", "", "", "fileServer", 65535, "R/MKDIR-FAIL: %s (%s)", newDirFull.c_str(), ec.message().c_str());
        getResponseContentStreamableObject()->writeString("false");
        return respond(HTTP::Status::Code::S_403_FORBIDDEN, clientRequest.getURI().c_str());
    }

    //////////////////////////////////////////////////////////////////////////
    if ((resolveLocalFilePathFromURI2(webClientParameters.httpDocumentRootDir, {}, &fileInfo, ".html", webClientParameters.execute)
         || resolveLocalFilePathFromURI2(webClientParameters.httpDocumentRootDir, {}, &fileInfo, "index.html", webClientParameters.execute)
         || resolveLocalFilePathFromURI2(webClientParameters.httpDocumentRootDir, {}, &fileInfo, "/index.html", webClientParameters.execute)
         || resolveLocalFilePathFromURI2(webClientParameters.httpDocumentRootDir, {}, &fileInfo, "", webClientParameters.execute))
        && !fileInfo.isDirectory)
    {
        // Evaluate...

        if (fileInfo.isExecutable)
        {
            if (clientRequest.requestLine.getHTTPMethod() == "POST" || clientRequest.requestLine.getHTTPMethod() == "GET")
            {
                // Execute...
                ret = HTTP::Status::Code::S_200_OK;

                std::string composedArgumentList, composedEnviromentList;
                Helpers::AppSpawn *spawner = new Helpers::AppSpawn();
                spawner->setExec(fileInfo.fullPath);

                spawner->addEnvironment(std::string("REMOTE_ADDR=") + clientRequest.networkClientInfo.REMOTE_ADDR);
                spawner->addEnvironment(std::string("REQUEST_METHOD=") + clientRequest.requestLine.getHTTPMethod());

                auto postVars = clientRequest.getVarsBySource(HTTP::Source::POST);
                for (const auto &key : postVars->getKeysList())
                {
                    // Security: Don't insert dangerous chars... (it should allow base64, emails, etc)
                    if (!containOnlyAllowedChars(key) || !containOnlyAllowedChars(postVars->getStringValue(key)))
                        continue;

                    spawner->addEnvironment("POST_" + key + "=" + postVars->getStringValue(key));
                    composedEnviromentList += (composedEnviromentList.empty() ? "" : " ") + std::string("POST_") + key;
                }

                auto getVars = clientRequest.getVarsBySource(HTTP::Source::GET);
                for (uint32_t i = 1; i < 1024 && getVars->getValue("a" + std::to_string(i)); i++)
                {
                    spawner->addArgument(getVars->getStringValue("a" + std::to_string(i)));
                    composedArgumentList += (composedArgumentList.empty() ? "" : " ") + getVars->getStringValue(std::string("a" + std::to_string(i)));
                }

                webClientParameters.rpcLog->log(Logs::DEBUG,
                                                clientRequest.networkClientInfo.REMOTE_ADDR,
                                                "",
                                                "",
                                                "",
                                                "fileServer",
                                                2048,
                                                "D/EXEC,%03d: %s (args: %s, env: %s)",
                                                static_cast<int>(ret),
                                                fileInfo.fullPath.c_str(),
                                                composedArgumentList.c_str(),
                                                composedEnviromentList.c_str());
                if (spawner->spawnProcess(true, false))
                {
                    webClientParameters.rpcLog->log(Logs::INFO,
                                                    clientRequest.networkClientInfo.REMOTE_ADDR,
                                                    "",
                                                    "",
                                                    "",
                                                    "fileServer",
                                                    2048,
                                                    "E/EXEC-OK,%03d: %s (args: %s, env: %s)",
                                                    static_cast<int>(ret),
                                                    fileInfo.relativePath.c_str(),
                                                    composedArgumentList.c_str(),
                                                    composedEnviromentList.c_str());
                    auto streamableP = std::make_shared<StreamableProcess>(spawner);
                    serverResponse.setContentDataStreamer(streamableP);
                    detectContentTypeFromFilePath(fileInfo.relativePath);
                }
                else
                {
                    webClientParameters.rpcLog->log(Logs::WARNING,
                                                    clientRequest.networkClientInfo.REMOTE_ADDR,
                                                    "",
                                                    "",
                                                    "",
                                                    "fileServer",
                                                    2048,
                                                    "E/EXEC-FAIL,%03d: %s (args: %s)",
                                                    static_cast<int>(ret),
                                                    fileInfo.relativePath.c_str(),
                                                    composedArgumentList.c_str());
                    delete spawner;
                }
            }
        }
        else
        {
            //Serve as it comes...
            ret = HTTP::Status::Code::S_200_OK;
            //            webClientParameters.rpcLog->log(Logs::INFO, clientRequest.networkClientInfo.REMOTE_ADDR,"", "","",  "fileServer", 2048, "R/%03d: %s",static_cast<int>(ret),fileInfo.relativePath.c_str());
            webClientParameters.rpcLog
                ->log(Logs::DEBUG, clientRequest.networkClientInfo.REMOTE_ADDR, "", "", "", "fileServer", 2048, "D/LOCAL,%03d: %s", static_cast<int>(ret), fileInfo.fullPath.c_str());
        }
    }
    else if (resolveLocalFilePathFromURI2(webClientParameters.httpDocumentRootDir, {}, &fileInfo) && fileInfo.isDirectory)
    {
        if (fileInfo.relativePath == "")
            fileInfo.relativePath = "/";
        ret = HTTP::Status::Code::S_200_OK;

        auto vars = clientRequest.getVarsBySource(HTTP::Source::GET);

        if (vars->getStringValue("action") == "targz" && webClientParameters.targz)
        {
            webClientParameters.rpcLog
                ->log(Logs::INFO, clientRequest.networkClientInfo.REMOTE_ADDR, "", "", "", "fileServer", 2048, "A/TARGZ-DIR,%03d: %s", static_cast<int>(ret), fileInfo.relativePath.c_str());
            generateTarGz(fileInfo);
        }
        else
        {
            //webClientParameters.rpcLog->log(Logs::INFO, clientRequest.networkClientInfo.REMOTE_ADDR,"", "","",  "fileServer", 2048, "I/DIR,%03d: %s",static_cast<int>(ret),fileInfo.relativePath.c_str());
            generateIndexOf(fileInfo);
        }
    }
    else
    {
        getResponseContentStreamableObject()->writeString(HTML_404);
    }

    if (fileInfo.relativePath.empty())
        return respond(ret, clientRequest.getURI());
    else
        return respond(ret, fileInfo.relativePath);
}

bool WebClientHdlr::containOnlyAllowedChars(const std::string &str)
{
    static const boost::regex allowedCharsRe("[a-zA-Z0-9.,!@_\\-+=/:]*");
    return boost::regex_match(str, allowedCharsRe);
}

std::string WebClientHdlr::sanitizeDownloadFileName(const std::string &name)
{
    static const boost::regex unsafeCharsRe("[^a-zA-Z0-9._\\-]");
    std::string safe = name;
    boost::replace_all_regex(safe, unsafeCharsRe, std::string("_"));
    return safe;
}
void WebClientHdlr::generateTarGz(const LocalRequestedFileInfo &fileInfo)
{
    serverResponse.contentType = "application/octet-stream";

    // Derive the download filename from the requested directory name (sanitized).
    const std::string dirName = fs::path(fileInfo.fullPath).filename().string();
    std::string fileName = sanitizeDownloadFileName(dirName) + ".tar.gz";
    if (fileName.size() <= 8)
        fileName = "archive.tar.gz"; // e.g. doc root "/" -> empty dir name

    serverResponse.headers.add("Content-Disposition", "attachment; filename=\"" + fileName + "\"");

    std::string composedArgumentList;
    Helpers::AppSpawn *spawner = new Helpers::AppSpawn();

    spawner->setExec("/usr/bin/tar");
    spawner->addArgument("czf");             // Compress (gzip), and put in file format.
    spawner->addArgument("-");               // Output to stdout (here)
    spawner->addArgument("--directory");     // chdir to the requested dir, so the archive contains only relative paths (no FS layout leak)
    spawner->addArgument(fileInfo.fullPath); // Our magic dir...
    spawner->addArgument(".");               // Archive the contents of that dir (relative), not the full absolute path

    if (spawner->spawnProcess(true, false))
    {
        auto streamableP = std::make_shared<StreamableProcess>(spawner);
        serverResponse.setContentDataStreamer(streamableP);
    }
    else
    {
        webClientParameters.rpcLog->log(Logs::WARNING, clientRequest.networkClientInfo.REMOTE_ADDR, "", "", "", "fileServer", 2048, "R/EXEC-FAIL: tar czvf");
        delete spawner;
    }
}

void WebClientHdlr::generateIndexOf(const LocalRequestedFileInfo &fileInfo)
{
    serverResponse.contentType = "text/html";

    getResponseContentStreamableObject()
        ->strPrintf(HTML_INDEX_HEADER,
                    htmlEncode(fileInfo.relativePath).c_str(),
                    HTML_CSS_PAGE,
                    (std::string(HTML_JS_COMMON) + HTML_JS_UPLOAD + (webClientParameters.overwrite ? std::string(HTML_JS_DELETE) + HTML_JS_MKDIR : std::string(""))).c_str(),
                    htmlEncode(fileInfo.relativePath).c_str(),
                    webClientParameters.targz ? HTML_TARGZ_LINK : "",
                    (std::string(webClientParameters.uploads ? HTML_UPLOAD_FORM : "") + (webClientParameters.overwrite ? HTML_MKDIR_FORM : "")).c_str(),
                    webClientParameters.overwrite ? HTML_INDEX_DELETE_HEADER_COL : "");

    std::error_code ec;

    if (!fs::is_directory(fileInfo.fullPath, ec) || ec)
    {
        getResponseContentStreamableObject()->writeString(HTML_INDEX_FOLDER_INACCESSIBLE);
    }
    else
    {
        // Show a parent ("..") link when the current directory is not the document root.
        if (fileInfo.relativePath != "/")
        {
            getResponseContentStreamableObject()->strPrintf(HTML_INDEX_ROW, HTML_ICON_FOLDER_PREFIX, "../", "..", HTML_ICON_FOLDER_SUFFIX, "", "", 0, 0, "", "");
        }

        std::vector<fs::directory_entry> dirEntries;
        for (auto it = fs::directory_iterator(fileInfo.fullPath, ec); !ec && it != fs::directory_iterator(); it.increment(ec))
            dirEntries.push_back(*it);

        // Sort: directories first, then case-insensitive by name.
        std::sort(dirEntries.begin(),
                  dirEntries.end(),
                  [](const fs::directory_entry &a, const fs::directory_entry &b)
                  {
                      const bool aDir = a.is_directory(), bDir = b.is_directory();
                      if (aDir != bDir)
                          return aDir;

                      std::string an = a.path().filename().string();
                      std::string bn = b.path().filename().string();
                      std::transform(an.begin(), an.end(), an.begin(), [](unsigned char c) { return std::tolower(c); });
                      std::transform(bn.begin(), bn.end(), bn.begin(), [](unsigned char c) { return std::tolower(c); });
                      return an < bn;
                  });

        for (const fs::directory_entry &entry : dirEntries)
        {
            const std::string name = entry.path().filename().string();

            std::error_code sec;
            const fs::file_status st = entry.status(sec);
            if (sec)
                continue;

            const bool isDir = (st.type() == fs::file_type::directory);

            // st_uid / st_gid are not exposed by std::filesystem; keep a plain stat for them (and mtime).
            struct stat64 rawStat;
            if (stat64(entry.path().c_str(), &rawStat) != 0)
                rawStat.st_uid = rawStat.st_gid = 0;

            time_t t = rawStat.st_mtime;
            struct tm lt;
            localtime_r(&t, &lt);
            char timebuf_mtime[128];
            strftime(timebuf_mtime, sizeof(timebuf_mtime), "%c", &lt);

            const fs::perms perms = st.permissions();
            int statchmod = static_cast<int>(perms & fs::perms::all);
            char chmodval[128];

            const fs::perms others = perms & fs::perms::others_all;
            if (others == (fs::perms::others_write | fs::perms::others_exec) || others == (fs::perms::others_read | fs::perms::others_write | fs::perms::others_exec))
            {
                snprintf(chmodval, sizeof(chmodval), HTML_CHMOD_WARNING, statchmod);
            }
            else
            {
                snprintf(chmodval, sizeof(chmodval), HTML_CHMOD_NORMAL, statchmod);
            }

            const std::string encName = Memory::Streams::Encoders::URL::encodeURLStr(name);
            // The delete endpoint is the directory containing the entry ("./.delete"),
            // mirroring the ".upload"/".dontupload" marker-file convention.
            const std::string delHref = std::string("./.delete?e=") + encName;
            std::string delCell;
            if (webClientParameters.overwrite)
            {
                char delBuf[512];
                if (isDir)
                    snprintf(delBuf, sizeof(delBuf), HTML_INDEX_DELETE_CELL_DIR, delHref.c_str(), htmlEncode(name).c_str());
                else
                    snprintf(delBuf, sizeof(delBuf), HTML_INDEX_DELETE_CELL_FILE, delHref.c_str(), htmlEncode(name).c_str());
                delCell = delBuf;
            }

            getResponseContentStreamableObject()->strPrintf(HTML_INDEX_ROW,
                                                            isDir ? HTML_ICON_FOLDER_PREFIX : HTML_ICON_FILE_PREFIX,
                                                            (encName + (isDir ? "/" : "")).c_str(),
                                                            htmlEncode(name).c_str(),
                                                            isDir ? HTML_ICON_FOLDER_SUFFIX : HTML_ICON_FILE_SUFFIX,
                                                            getFancySize(static_cast<size_t>(rawStat.st_size)).c_str(),
                                                            timebuf_mtime,
                                                            rawStat.st_uid,
                                                            rawStat.st_gid,
                                                            chmodval,
                                                            delCell.c_str());
        }
        getResponseContentStreamableObject()->writeString(HTML_INDEX_TABLE_CLOSE);
    }

    getResponseContentStreamableObject()->strPrintf(HTML_INDEX_FOOTER, webClientParameters.softwareVersion.c_str());
}

Mantids30::Network::Protocol::HTTP::Status::Code WebClientHdlr::handleDeleteRequest(const LocalRequestedFileInfo &fileInfo)
{
    // The document root itself is never deletable (guaranteed by the caller: relativePath != "").
    std::error_code rcec;

    if (fs::is_directory(fileInfo.fullPath))
    {
        fs::remove_all(fileInfo.fullPath, rcec);
    }
    else
    {
        fs::remove(fileInfo.fullPath, rcec);
    }

    if (rcec)
    {
        webClientParameters.rpcLog
            ->log(Logs::ERROR, clientRequest.networkClientInfo.REMOTE_ADDR, "", "", "", "fileServer", 65535, "R/DELETE-FAIL: %s (%s)", fileInfo.relativePath.c_str(), rcec.message().c_str());
        getResponseContentStreamableObject()->writeString("false");
        return respond(HTTP::Status::Code::S_403_FORBIDDEN, fileInfo.relativePath);
    }

    webClientParameters.rpcLog->log(Logs::INFO, clientRequest.networkClientInfo.REMOTE_ADDR, "", "", "", "fileServer", 65535, "R/DELETE-OK: %s", fileInfo.relativePath.c_str());
    getResponseContentStreamableObject()->writeString("true");
    return respond(HTTP::Status::Code::S_200_OK, fileInfo.relativePath);
}

std::string WebClientHdlr::getFancySize(size_t value)
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
        sprintf(output, "%.1f%s\n", (double) value + ((double) remainder / (double) 1024), szNames[pos]);
    else
        sprintf(output, "%zu%s\n", value, szNames[pos]);
    return std::string(output);
}

void WebClientHdlr::setWebClientParameters(const webClientParams &newWebClientParameters)
{
    webClientParameters = newWebClientParameters;
}
