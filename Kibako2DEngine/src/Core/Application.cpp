#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#define SDL_MAIN_HANDLED

#include "KibakoEngine/Core/Application.h"

#include "KibakoEngine/Core/Debug.h"
#include "KibakoEngine/Core/Log.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_syswm.h>

#include <windows.h>

namespace KibakoEngine {

    bool Application::CreateWindowSDL(int width, int height, const char* title)
    {
        SDL_SetMainReady();
        if (SDL_Init(SDL_INIT_VIDEO) != 0) {
            KbkLog("App", "SDL_Init failed: %s", SDL_GetError());
            return false;
        }

        m_window = SDL_CreateWindow(title,
                                    SDL_WINDOWPOS_CENTERED,
                                    SDL_WINDOWPOS_CENTERED,
                                    width,
                                    height,
                                    SDL_WINDOW_RESIZABLE | SDL_WINDOW_SHOWN);
        if (!m_window) {
            KbkLog("App", "SDL_CreateWindow failed: %s", SDL_GetError());
            SDL_Quit();
            return false;
        }

        SDL_SysWMinfo info{};
        SDL_VERSION(&info.version);
        if (SDL_GetWindowWMInfo(m_window, &info) == SDL_FALSE) {
            KbkLog("App", "SDL_GetWindowWMInfo failed: %s", SDL_GetError());
            SDL_DestroyWindow(m_window);
            m_window = nullptr;
            SDL_Quit();
            return false;
        }

        m_hwnd = info.info.win.window;
        KBK_ASSERT(m_hwnd != nullptr, "SDL window did not provide a valid HWND");
        return true;
    }

    void Application::DestroyWindowSDL()
    {
        if (m_window) {
            SDL_DestroyWindow(m_window);
            m_window = nullptr;
        }
        SDL_Quit();
        m_hwnd = nullptr;
    }

    void Application::HandleResize()
    {
        if (!m_window)
            return;

        int drawableW = 0;
        int drawableH = 0;
        SDL_GetWindowSizeInPixels(m_window, &drawableW, &drawableH);
        if (drawableW <= 0 || drawableH <= 0)
            return;

        if (drawableW == m_width && drawableH == m_height)
            return;

        m_width = drawableW;
        m_height = drawableH;
        KbkLog("App", "Resize -> %dx%d", m_width, m_height);
        m_renderer.OnResize(static_cast<uint32_t>(m_width), static_cast<uint32_t>(m_height));
    }

    bool Application::Init(int width, int height, const char* title)
    {
        if (m_running)
            return true;

        if (!CreateWindowSDL(width, height, title))
            return false;

        SDL_GetWindowSizeInPixels(m_window, &m_width, &m_height);
        KbkLog("App", "Drawable size: %dx%d", m_width, m_height);

        if (!m_renderer.Init(m_hwnd, static_cast<uint32_t>(m_width), static_cast<uint32_t>(m_height))) {
            Shutdown();
            return false;
        }

        m_running = true;
        return true;
    }

    void Application::Shutdown()
    {
        if (!m_running)
            return;

        m_renderer.Shutdown();
        DestroyWindowSDL();
        m_running = false;
    }

    bool Application::PumpEvents()
    {
        if (!m_running)
            return false;

        m_input.BeginFrame();
        m_time.Tick();

        SDL_Event evt{};
        while (SDL_PollEvent(&evt)) {
            switch (evt.type) {
            case SDL_QUIT:
                return false;
            case SDL_WINDOWEVENT:
                if (evt.window.event == SDL_WINDOWEVENT_SIZE_CHANGED ||
                    evt.window.event == SDL_WINDOWEVENT_RESIZED ||
#ifdef SDL_WINDOWEVENT_DISPLAY_SCALE_CHANGED
                    evt.window.event == SDL_WINDOWEVENT_DISPLAY_SCALE_CHANGED ||
#endif
                    evt.window.event == SDL_WINDOWEVENT_MAXIMIZED ||
                    evt.window.event == SDL_WINDOWEVENT_RESTORED) {
                    HandleResize();
                }
                break;
            case SDL_KEYDOWN:
                if (evt.key.keysym.scancode == SDL_SCANCODE_ESCAPE)
                    return false;
                break;
            default:
                break;
            }

            m_input.HandleEvent(evt);
        }

        return true;
    }

    void Application::BeginFrame(const float clearColor[4])
    {
        m_renderer.BeginFrame(clearColor);
    }

    void Application::EndFrame(bool waitForVSync)
    {
        m_renderer.EndFrame(waitForVSync);
        m_input.EndFrame();
    }

} // namespace KibakoEngine

