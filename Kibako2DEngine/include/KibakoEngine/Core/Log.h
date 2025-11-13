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

#define KBK_DETAIL_LOG(level, channel, ...)                                                                    \
    ::KibakoEngine::Detail::LogMessageForward(::KibakoEngine::LogLevel::level, (channel), __VA_ARGS__)

#define KbkLog(channel, ...)  KBK_DETAIL_LOG(Info, (channel), __VA_ARGS__)
#define KbkWarn(channel, ...) KBK_DETAIL_LOG(Warning, (channel), __VA_ARGS__)
#define KbkError(channel, ...) KBK_DETAIL_LOG(Error, (channel), __VA_ARGS__)

#define KbkLogDefault(...)  KbkLog(KBK_LOG_CHANNEL_DEFAULT, __VA_ARGS__)
#define KbkWarnDefault(...) KbkWarn(KBK_LOG_CHANNEL_DEFAULT, __VA_ARGS__)
#define KbkErrorDefault(...) KbkError(KBK_LOG_CHANNEL_DEFAULT, __VA_ARGS__)

