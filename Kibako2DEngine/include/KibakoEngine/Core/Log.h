#pragma once

#include <cstdarg>
#include <cstdint>
#include <utility>

namespace KibakoEngine {

    enum class LogLevel : std::uint8_t
    {
        Trace,
        Info,
        Warning,
        Error,
        Critical
    };

    struct LogConfig
    {
        LogLevel minimumLevel = LogLevel::Trace;
        LogLevel debuggerBreakLevel = LogLevel::Error;
        bool breakIntoDebugger = true;
        bool haltRenderingOnBreak = true;
    };

    void SetLogConfig(const LogConfig& config);
    LogConfig GetLogConfig();

    void RequestBreakpoint(const char* reason, LogLevel level = LogLevel::Error);

    bool HasBreakpointRequest();
    bool ConsumeBreakpointRequest();
    const char* LastBreakpointMessage();

    void LogMessage(LogLevel level,
                    const char* channel,
                    const char* file,
                    int line,
                    const char* function,
                    const char* fmt,
                    ...);
    void LogMessageV(LogLevel level,
                     const char* channel,
                     const char* file,
                     int line,
                     const char* function,
                     const char* fmt,
                     std::va_list args);

    namespace Detail
    {
        template <typename... Args>
        inline void LogMessageForward(LogLevel level,
                                      const char* channel,
                                      const char* file,
                                      int line,
                                      const char* function,
                                      const char* fmt,
                                      Args&&... args)
        {
            LogMessage(level,
                       channel,
                       file,
                       line,
                       function,
                       fmt,
                       std::forward<Args>(args)...);
        }
    } // namespace Detail

} // namespace KibakoEngine

#define KBK_LOG_CHANNEL_DEFAULT "Kibako"

#define KBK_LOG(level, channel, fmt, ...)                                                                        \
    ::KibakoEngine::Detail::LogMessageForward((level),                                                          \
                                              (channel),                                                        \
                                              __FILE__,                                                         \
                                              __LINE__,                                                         \
                                              __func__,                                                         \
                                              (fmt) __VA_OPT__(, ) __VA_ARGS__)

#define KbkTrace(channel, fmt, ...)                                                                            \
    KBK_LOG(::KibakoEngine::LogLevel::Trace, (channel), (fmt) __VA_OPT__(, ) __VA_ARGS__)
#define KbkLog(channel, fmt, ...)                                                                              \
    KBK_LOG(::KibakoEngine::LogLevel::Info, (channel), (fmt) __VA_OPT__(, ) __VA_ARGS__)
#define KbkWarn(channel, fmt, ...)                                                                             \
    KBK_LOG(::KibakoEngine::LogLevel::Warning, (channel), (fmt) __VA_OPT__(, ) __VA_ARGS__)
#define KbkError(channel, fmt, ...)                                                                            \
    KBK_LOG(::KibakoEngine::LogLevel::Error, (channel), (fmt) __VA_OPT__(, ) __VA_ARGS__)
#define KbkCritical(channel, fmt, ...)                                                                         \
    KBK_LOG(::KibakoEngine::LogLevel::Critical, (channel), (fmt) __VA_OPT__(, ) __VA_ARGS__)

#define KbkLogDefault(fmt, ...)  KbkLog(KBK_LOG_CHANNEL_DEFAULT, fmt __VA_OPT__(, ) __VA_ARGS__)
#define KbkWarnDefault(fmt, ...) KbkWarn(KBK_LOG_CHANNEL_DEFAULT, fmt __VA_OPT__(, ) __VA_ARGS__)
#define KbkErrorDefault(fmt, ...) KbkError(KBK_LOG_CHANNEL_DEFAULT, fmt __VA_OPT__(, ) __VA_ARGS__)
#define KbkCriticalDefault(fmt, ...) KbkCritical(KBK_LOG_CHANNEL_DEFAULT, fmt __VA_OPT__(, ) __VA_ARGS__)

