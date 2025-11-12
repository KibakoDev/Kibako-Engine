#pragma once

// Minimal debug helpers used across the engine. All macros become no-ops in
// release builds but still validate expressions in debug configurations.

#include <cstdio>
#include <cstdlib>

#ifdef _WIN32
#    include <windows.h>
#endif

#if defined(_MSC_VER)
#    define KBK_BREAK() __debugbreak()
#else
#    include <csignal>
#    define KBK_BREAK() std::raise(SIGTRAP)
#endif

#if defined(_DEBUG)
#    define KBK_ASSERT(cond, msg)                                                                  \
        do {                                                                                       \
            if (!(cond)) {                                                                         \
                std::fprintf(stderr, "[KBK_ASSERT] %s\nFile: %s(%d)\n", (msg), __FILE__, __LINE__); \
                std::fflush(stderr);                                                               \
                KBK_BREAK();                                                                       \
            }                                                                                      \
        } while (0)
#else
#    define KBK_ASSERT(cond, msg) ((void)sizeof(cond))
#endif

#if defined(_DEBUG)
#    define KBK_HR(expr)                                                                           \
        do {                                                                                       \
            const HRESULT _kbk_hr = (expr);                                                        \
            if (FAILED(_kbk_hr)) {                                                                 \
                std::fprintf(stderr,                                                               \
                             "[KBK_HR] hr=0x%08X\nExpr: %s\nFile: %s(%d)\n",                      \
                             static_cast<unsigned>(_kbk_hr),                                       \
                             #expr,                                                                \
                             __FILE__,                                                             \
                             __LINE__);                                                            \
                std::fflush(stderr);                                                               \
                KBK_BREAK();                                                                       \
            }                                                                                      \
        } while (0)
#else
#    define KBK_HR(expr) ((void)(expr))
#endif

