#pragma once

#include <cstdarg>
#include <cstdint>

namespace KibakoEngine {

    enum class LogLevel : std::uint8_t
    {
        Info,
        Warning,
        Error
    };

    void LogMessage(LogLevel level, const char* channel, const char* fmt, ...);
    void LogMessageV(LogLevel level, const char* channel, const char* fmt, std::va_list args);

} // namespace KibakoEngine

#define KBK_LOG_CHANNEL_DEFAULT "Kibako"

#define KbkLog(...) \
    ::KibakoEngine::LogMessage(::KibakoEngine::LogLevel::Info, __VA_ARGS__)
#define KbkWarn(...) \
    ::KibakoEngine::LogMessage(::KibakoEngine::LogLevel::Warning, __VA_ARGS__)
#define KbkError(...) \
    ::KibakoEngine::LogMessage(::KibakoEngine::LogLevel::Error, __VA_ARGS__)
#define KbkLog(channel, fmt, ...) \
    ::KibakoEngine::LogMessage(::KibakoEngine::LogLevel::Info, (channel), (fmt) __VA_OPT__(, ) __VA_ARGS__)
#define KbkWarn(channel, fmt, ...) \
    ::KibakoEngine::LogMessage(::KibakoEngine::LogLevel::Warning, (channel), (fmt) __VA_OPT__(, ) __VA_ARGS__)
#define KbkError(channel, fmt, ...) \
    ::KibakoEngine::LogMessage(::KibakoEngine::LogLevel::Error, (channel), (fmt) __VA_OPT__(, ) __VA_ARGS__)

