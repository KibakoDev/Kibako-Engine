#pragma once
#include <cstdarg>
#include <cstdio>
#ifdef _WIN32
#include <windows.h>
#endif

// Simple logging helper: prints to stdout and VS Output window (Debug).
// Usage: KbkLog("Renderer", "Created device: %p", device);
inline void KbkLog(const char* tag, const char* fmt, ...)
{
    char buffer[1024];
    int n = 0;
    if (tag && *tag)
        n = std::snprintf(buffer, sizeof(buffer), "[%s] ", tag);

    va_list args; va_start(args, fmt);
    std::vsnprintf(buffer + n, sizeof(buffer) - (size_t)n, fmt, args);
    va_end(args);

    std::fputs(buffer, stdout);
    std::fputc('\n', stdout);
    std::fflush(stdout);

#ifdef _WIN32
    // Mirror to Visual Studio Output panel
    OutputDebugStringA(buffer);
    OutputDebugStringA("\n");
#endif
}