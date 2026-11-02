#pragma once

#include <Mantids30/Program_Logs/loglevels.h>
#include <Mantids30/Program_Logs/rpclog.h>

#include <cstdarg>
#include <string>

namespace uPocketWS::Log {
inline constexpr const char *MODULE = "fileServer";
inline constexpr uint32_t FULL_SIZE = 65535;
inline constexpr uint32_t SHORT_SIZE = 2048;

/**
     * @brief logRpc Format and emit a structured RPC log line.
     *        Replaces the repeated rpcLog->log(level, ip, "", "", "", "fileServer", size, fmt, ...) calls.
     */
template<typename... Args>
void logRpc(Mantids30::Program::Logs::RPCLog *rpcLog, Mantids30::Program::Logs::LogLevel level, const std::string &remoteIP, uint32_t outSize, const char *fmt, Args &&...args)
{
    if (rpcLog != nullptr)
        rpcLog->log(level, remoteIP, "", "", "", MODULE, outSize, fmt, std::forward<Args>(args)...);
}
} // namespace uPocketWS::Log
