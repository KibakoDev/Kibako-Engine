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

#define KBK_LOG(level, channel, ...)                                                                           \
    ::KibakoEngine::Detail::LogMessageForward((level),                                                         \
                                              (channel),                                                       \
                                              __FILE__,                                                        \
                                              __LINE__,                                                        \
                                              __func__,                                                        \
                                              __VA_ARGS__)

#define KbkTrace(channel, ...)   KBK_LOG(::KibakoEngine::LogLevel::Trace, (channel), __VA_ARGS__)
#define KbkLog(channel, ...)     KBK_LOG(::KibakoEngine::LogLevel::Info, (channel), __VA_ARGS__)
#define KbkWarn(channel, ...)    KBK_LOG(::KibakoEngine::LogLevel::Warning, (channel), __VA_ARGS__)
#define KbkError(channel, ...)   KBK_LOG(::KibakoEngine::LogLevel::Error, (channel), __VA_ARGS__)
#define KbkCritical(channel, ...)                                                                              \
    KBK_LOG(::KibakoEngine::LogLevel::Critical, (channel), __VA_ARGS__)

#define KbkLogDefault(...)      KbkLog(KBK_LOG_CHANNEL_DEFAULT, __VA_ARGS__)
#define KbkWarnDefault(...)     KbkWarn(KBK_LOG_CHANNEL_DEFAULT, __VA_ARGS__)
#define KbkErrorDefault(...)    KbkError(KBK_LOG_CHANNEL_DEFAULT, __VA_ARGS__)
#define KbkCriticalDefault(...) KbkCritical(KBK_LOG_CHANNEL_DEFAULT, __VA_ARGS__)

