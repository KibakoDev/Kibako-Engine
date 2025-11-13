#include "KibakoEngine/Core/Log.h"

#include <array>
#include <cstdio>
#include <cstring>
#include <mutex>

#ifdef _WIN32
#    include <windows.h>
#endif

namespace KibakoEngine {

    namespace
    {
        constexpr std::size_t kLogBufferSize = 1024;

        const char* LevelPrefix(LogLevel level)
        {
            switch (level) {
            case LogLevel::Info: return "[Info]";
            case LogLevel::Warning: return "[Warn]";
            case LogLevel::Error: return "[Error]";
            default: return "[Log]";
            }
        }

        void OutputToDebugger(const char* text)
        {
#ifdef _WIN32
            OutputDebugStringA(text);
#endif
        }

        std::mutex& LogMutex()
        {
            static std::mutex s_mutex;
            return s_mutex;
        }
    } // namespace

    void LogMessageV(LogLevel level, const char* channel, const char* fmt, std::va_list args)
    {
        std::lock_guard<std::mutex> guard(LogMutex());

        std::array<char, kLogBufferSize> buffer{};
        const char* levelPrefix = LevelPrefix(level);
        int written = std::snprintf(buffer.data(), buffer.size(), "%s", levelPrefix);
        if (written < 0)
            return;

        std::size_t offset = static_cast<std::size_t>(written);
        if (channel && channel[0] != '\0') {
            written = std::snprintf(buffer.data() + offset, buffer.size() - offset, "[%s] ", channel);
            if (written < 0)
                return;
            offset += static_cast<std::size_t>(written);
        }

        written = std::vsnprintf(buffer.data() + offset, buffer.size() - offset, fmt, args);
        if (written < 0)
            return;
        offset += static_cast<std::size_t>(written);

        if (offset + 1 < buffer.size())
            buffer[offset++] = '\n';
        buffer[offset] = '\0';

        std::fputs(buffer.data(), (level == LogLevel::Error) ? stderr : stdout);
        std::fflush((level == LogLevel::Error) ? stderr : stdout);
        OutputToDebugger(buffer.data());
    }

    void LogMessage(LogLevel level, const char* channel, const char* fmt, ...)
    {
        std::va_list args;
        va_start(args, fmt);
        LogMessageV(level, channel, fmt, args);
        va_end(args);
    }

} // namespace KibakoEngine

