#pragma once

#include <cstdarg>
#include <cstdint>
#include <utility>

namespace KibakoEngine {

    enum class LogLevel : std::uint8_t
    {
        Info,
        Warning,
        Error
    };

    void LogMessage(LogLevel level, const char* channel, const char* fmt, ...);
    void LogMessageV(LogLevel level, const char* channel, const char* fmt, std::va_list args);

    namespace Detail
    {
        template <typename... Args>
        inline void LogMessageForward(LogLevel level, const char* channel, const char* fmt, Args&&... args)
        {
            LogMessage(level, channel, fmt, std::forward<Args>(args)...);
        }
    } // namespace Detail

} // namespace KibakoEngine

#define KBK_LOG_CHANNEL_DEFAULT "Kibako"

#define KbkLog(channel, fmt, ...)                                                                             \
    ::KibakoEngine::LogMessage(::KibakoEngine::LogLevel::Info, (channel), (fmt) __VA_OPT__(, ) __VA_ARGS__)
#define KbkWarn(channel, fmt, ...)                                                                            \
    ::KibakoEngine::LogMessage(::KibakoEngine::LogLevel::Warning, (channel), (fmt) __VA_OPT__(, ) __VA_ARGS__)
#define KbkError(channel, fmt, ...)                                                                           \
    ::KibakoEngine::LogMessage(::KibakoEngine::LogLevel::Error, (channel), (fmt) __VA_OPT__(, ) __VA_ARGS__)

#define KbkLogDefault(fmt, ...)  KbkLog(KBK_LOG_CHANNEL_DEFAULT, fmt __VA_OPT__(, ) __VA_ARGS__)
#define KbkWarnDefault(fmt, ...) KbkWarn(KBK_LOG_CHANNEL_DEFAULT, fmt __VA_OPT__(, ) __VA_ARGS__)
#define KbkErrorDefault(fmt, ...) KbkError(KBK_LOG_CHANNEL_DEFAULT, fmt __VA_OPT__(, ) __VA_ARGS__)

