#include "utils/webutils.h"
#include "web/handlers/webhandlers.h"
#include "web/webclienthdlr.h"
#include "web/weblog.h"

#include <Mantids30/Helpers/appexec.h>
#include <Mantids30/Memory/streamable_process.h>

using namespace Mantids30;
using namespace Mantids30::Network::Protocol;
using namespace Mantids30::Program;

namespace uPocketWS::Web::Handlers {
// Executable files, only with --can-execute.
bool ExecHandler::canHandle(WebClientHdlr &srv, const WebRequestContext &ctx) const
{
    const std::string method = srv.clientRequest.requestLine.getHTTPMethod();
    return srv.params().execute && !ctx.fileInfo.isDirectory && ctx.fileInfo.isExecutable && (method == "GET" || method == "POST");
}

IWebRequestHandler::Code ExecHandler::handle(WebClientHdlr &srv, WebRequestContext &ctx)
{
    const webClientParams &cfg = srv.params();
    const auto &fileInfo = ctx.fileInfo;
    const Code ret = HTTP::Status::Code::S_200_OK;

    std::string composedArgumentList, composedEnvironmentList;
    auto *spawner = new Helpers::AppSpawn();
    spawner->setExec(fileInfo.fullPath);
    spawner->addEnvironment("REMOTE_ADDR=" + srv.remoteIP());
    spawner->addEnvironment("REQUEST_METHOD=" + srv.clientRequest.requestLine.getHTTPMethod());

    // POST vars -> POST_<key> environment (only safe charset).
    if (auto postVars = srv.clientRequest.getVarsBySource(HTTP::Source::POST))
    {
        for (const auto &key : postVars->getKeysList())
        {
            const std::string value = postVars->getStringValue(key);
            if (!Utils::containOnlyAllowedChars(key) || !Utils::containOnlyAllowedChars(value))
                continue;
            spawner->addEnvironment("POST_" + key + "=" + value);
            composedEnvironmentList += (composedEnvironmentList.empty() ? "" : " ") + std::string("POST_") + key;
        }
    }

    // GET vars a1..a1023 -> positional arguments.
    if (auto getVars = srv.clientRequest.getVarsBySource(HTTP::Source::GET))
    {
        for (uint32_t i = 1; i < 1024 && getVars->getValue("a" + std::to_string(i)); i++)
        {
            const std::string arg = getVars->getStringValue("a" + std::to_string(i));
            spawner->addArgument(arg);
            composedArgumentList += (composedArgumentList.empty() ? "" : " ") + arg;
        }
    }

    Log::logRpc(cfg.rpcLog,
                Logs::DEBUG,
                srv.remoteIP(),
                Log::SHORT_SIZE,
                "D/EXEC,%03d: %s (args: %s, env: %s)",
                static_cast<int>(ret),
                fileInfo.fullPath.c_str(),
                composedArgumentList.c_str(),
                composedEnvironmentList.c_str());

    if (spawner->spawnProcess(true, false))
    {
        Log::logRpc(cfg.rpcLog,
                    Logs::INFO,
                    srv.remoteIP(),
                    Log::SHORT_SIZE,
                    "E/EXEC-OK,%03d: %s (args: %s, env: %s)",
                    static_cast<int>(ret),
                    fileInfo.relativePath.c_str(),
                    composedArgumentList.c_str(),
                    composedEnvironmentList.c_str());
        srv.serverResponse.setContentDataStreamer(std::make_shared<Memory::Streams::StreamableProcess>(spawner));
        srv.detectContentType(fileInfo.relativePath);
    }
    else
    {
        Log::logRpc(cfg.rpcLog, Logs::WARNING, srv.remoteIP(), Log::SHORT_SIZE, "E/EXEC-FAIL,%03d: %s (args: %s)", static_cast<int>(ret), fileInfo.relativePath.c_str(), composedArgumentList.c_str());
        delete spawner;
    }
    return srv.respond(ret, fileInfo.relativePath);
}
} // namespace uPocketWS::Web::Handlers
