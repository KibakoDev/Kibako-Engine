// Kibako2DEngine/Core/Application.cpp
// Creates the window and keeps the frame flow simple.

#define WIN32_LEAN_AND_MEAN
#define SDL_MAIN_HANDLED

#include <windows.h>
#include <iostream>

#include <SDL2/SDL.h>
#include <SDL2/SDL_syswm.h>

#include "KibakoEngine/Core/Application.h"
#include "KibakoEngine/Core/Debug.h"
#include "KibakoEngine/Core/Log.h"

namespace KibakoEngine {

    // -----------------------
    // Window creation / tear
    // -----------------------
    bool Application::CreateWindowSDL(int w, int h, const char* title)
    {
        SDL_SetMainReady();
        if (SDL_Init(SDL_INIT_VIDEO) != 0) {
            std::cerr << "SDL_Init Error: " << SDL_GetError() << "\n";
            return false;
        }

        KbkLog("App", "Creating window %dx%d: %s", w, h, title);

        m_window = SDL_CreateWindow(
            title,
            SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
            w, h,
            SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE /* | SDL_WINDOW_ALLOW_HIGHDPI */
        );
        if (!m_window) {
            std::cerr << "SDL_CreateWindow Error: " << SDL_GetError() << "\n";
            SDL_Quit();
            return false;
        }

        SDL_SysWMinfo wminfo{};
        SDL_VERSION(&wminfo.version);
        if (!SDL_GetWindowWMInfo(m_window, &wminfo)) {
            std::cerr << "SDL_GetWindowWMInfo failed" << std::endl;
            SDL_DestroyWindow(m_window);
            m_window = nullptr;
            SDL_Quit();
            return false;
        }

        m_hwnd = wminfo.info.win.window;
        KBK_ASSERT(m_hwnd != nullptr, "HWND is null after SDL_GetWindowWMInfo");

        m_width = w;
        m_height = h;
        return true;
    }

    void Application::DestroyWindowSDL()
    {
        if (m_window) {
            SDL_DestroyWindow(m_window);
            m_window = nullptr;
        }
        SDL_Quit();
        KbkLog("App", "SDL shutdown complete");
    }

    // -----------------------
    // Resize handling
    // -----------------------
    void Application::OnResize(int /*newWidth*/, int /*newHeight*/)
    {
        // Query true drawable size (handles DPI scaling)
        int pxW = 0, pxH = 0;
        SDL_GetWindowSizeInPixels(m_window, &pxW, &pxH);
        if (pxW <= 0 || pxH <= 0) return; // minimized / invalid

        m_width = pxW;
        m_height = pxH;

        KbkLog("App", "Resize -> %dx%d (drawable px)", m_width, m_height);

        // Notify renderer (resize swapchain, viewport, etc.)
        m_renderer.OnResize(m_width, m_height);
    }

    // -----------------------
    // Init / Shutdown
    // -----------------------
    bool Application::Init(int width, int height, const char* title)
    {
        if (m_running) return true;

        KbkLog("App", "Init start");

        // 1) Window
        if (!CreateWindowSDL(width, height, title))
            return false;

        // 2) True pixel size (DPI)
        SDL_GetWindowSizeInPixels(m_window, &m_width, &m_height);
        KbkLog("App", "Pixel size (DPI-aware): %dx%d", m_width, m_height);

        // 3) D3D11 renderer
        KBK_ASSERT(m_hwnd != nullptr, "Init without a valid HWND");
        if (!m_renderer.Init(m_hwnd, m_width, m_height))
            return false;

        m_running = true;
        KbkLog("App", "Init done");
        return true;
    }

    void Application::Shutdown()
    {
        if (!m_running) return;

        KbkLog("App", "Shutdown");
        m_renderer.Shutdown();
        DestroyWindowSDL();
        m_running = false;
    }

    // -----------------------
    // Frame control
    // -----------------------
    bool Application::PumpEvents()
    {
        if (!m_running) return false;

        m_input.BeginFrame();
        m_time.Tick();

        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            switch (e.type) {

            case SDL_QUIT:
                KbkLog("App", "Quit requested (SDL_QUIT)");
                return false;

            case SDL_KEYDOWN:
                if (e.key.keysym.sym == SDLK_ESCAPE) {
                    KbkLog("App", "Quit requested (ESC)");
                    return false;
                }
                break;

            case SDL_WINDOWEVENT:
                switch (e.window.event) {
                case SDL_WINDOWEVENT_SIZE_CHANGED:
                case SDL_WINDOWEVENT_RESIZED:
                    OnResize(e.window.data1, e.window.data2);
                    break;
#if defined(SDL_WINDOWEVENT_DISPLAY_SCALE_CHANGED)
                case SDL_WINDOWEVENT_DISPLAY_SCALE_CHANGED:
                    OnResize(0, 0); // re-fetch real pixel size
                    break;
#endif
                default: break;
                }
                break;

            default:
                break;
            }

            // Forward to input system
            m_input.HandleEvent(e);
        }

        return true;
    }

    void Application::BeginFrame()
    {
        // Renderer write begins
        m_renderer.BeginFrame();
    }

    void Application::EndFrame()
    {
        // Present + input frame end
        m_renderer.EndFrame();
        m_input.EndFrame();

        // Optional tiny FPS print each ~0.5s
        static double acc = 0.0;
        acc += m_time.DeltaSeconds();
        if (acc > 0.5) {
            std::cout << "FPS: " << (int)m_time.FPS() << "\r";
            acc = 0.0;
        }
    }

}