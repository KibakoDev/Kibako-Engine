#pragma once

#include <cstdarg>
#include <cstdio>
#include <cstring>

#ifdef _WIN32
#    include <windows.h>
#endif

namespace KibakoEngine {

    namespace Detail {
        inline unsigned HashTag(const char* tag)
        {
            unsigned h = 5381u;
            if (!tag) return h;
            while (*tag) {
                h = ((h << 5) + h) + static_cast<unsigned>(*tag++);
            }
            return h;
        }

    } // namespace Detail

    inline void KbkLog(const char* tag, const char* fmt, ...)
    {
        char buffer[1024];
        int prefixLen = 0;
        if (tag && *tag) {
            prefixLen = std::snprintf(buffer, sizeof(buffer), "[%s] ", tag);
        }

        va_list args;
        va_start(args, fmt);
        std::vsnprintf(buffer + prefixLen, sizeof(buffer) - static_cast<size_t>(prefixLen), fmt, args);
        va_end(args);

#ifdef _WIN32
        HANDLE hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
        CONSOLE_SCREEN_BUFFER_INFO oldInfo{};
        const bool hasConsole = GetConsoleScreenBufferInfo(hStdOut, &oldInfo) != 0;
        WORD color = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE;
        if (hasConsole && tag && *tag) {
            static const WORD palette[] = {
                WORD(FOREGROUND_RED | FOREGROUND_INTENSITY),
                WORD(FOREGROUND_GREEN | FOREGROUND_INTENSITY),
                WORD(FOREGROUND_BLUE | FOREGROUND_INTENSITY),
                WORD(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY),
                WORD(FOREGROUND_RED | FOREGROUND_BLUE | FOREGROUND_INTENSITY),
                WORD(FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY),
                WORD(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE)
            };
            color = palette[Detail::HashTag(tag) % (sizeof(palette) / sizeof(palette[0]))];
            SetConsoleTextAttribute(hStdOut, color);
        }
        if (hasConsole)
            std::fputs(buffer, stdout);
        else
            std::printf("%s", buffer);
        std::fputc('\n', stdout);
        std::fflush(stdout);
        if (hasConsole)
            SetConsoleTextAttribute(hStdOut, oldInfo.wAttributes);
        OutputDebugStringA(buffer);
        OutputDebugStringA("\n");
#else
        std::fputs(buffer, stdout);
        std::fputc('\n', stdout);
        std::fflush(stdout);
#endif
    }

} // namespace KibakoEngine

