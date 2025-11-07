#pragma once
#include <cstdio>
#include <cstdlib>
#ifdef _WIN32
#include <windows.h>
#endif

// --- Break helper ---
#if defined(_MSC_VER)
#define KBK_BREAK() __debugbreak()
#else
#include <csignal>
#define KBK_BREAK() std::raise(SIGTRAP)
#endif

// --- Assertion: active only in Debug ---
#if defined(_DEBUG)
#define KBK_ASSERT(cond, msg)                                                \
    do {                                                                       \
        if (!(cond)) {                                                         \
            std::fprintf(stderr, "[ASSERT] %s\nFile: %s:%d\n", (msg), __FILE__, __LINE__); \
            std::fflush(stderr);                                               \
            KBK_BREAK();                                                       \
        }                                                                      \
    } while (0)
#else
#define KBK_ASSERT(cond, msg) do { (void)sizeof(cond); } while (0)
#endif

// --- HRESULT checker: active only in Debug ---
#if defined(_DEBUG)
#define KBK_HR(call)                                                         \
    do {                                                                       \
        HRESULT _hr_ = (call);                                                 \
        if (FAILED(_hr_)) {                                                    \
            std::fprintf(stderr, "[D3D11] Call failed (hr=0x%08X)\nExpr: %s\nFile: %s:%d\n", \
                         (unsigned)_hr_, #call, __FILE__, __LINE__);           \
            std::fflush(stderr);                                               \
            KBK_BREAK();                                                       \
        }                                                                      \
    } while (0)
#else
#define KBK_HR(call) do { (void)(call); } while (0)
#endif