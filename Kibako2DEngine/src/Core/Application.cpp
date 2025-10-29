// Kibako2DEngine/src/Core/Application.cpp
#define WIN32_LEAN_AND_MEAN
#define SDL_MAIN_HANDLED

#include <windows.h>
#include <iostream>

#include <SDL2/SDL.h>
#include <SDL2/SDL_syswm.h>

#include "KibakoEngine/Core/Application.h"

namespace KibakoEngine
{
    // ----------------------
    // Private helpers
    // ----------------------

    bool Application::CreateWindowSDL(int w, int h, const char* title)
    {
        SDL_SetMainReady();
        if (SDL_Init(SDL_INIT_VIDEO) != 0)
        {
            std::cerr << "SDL_Init Error: " << SDL_GetError() << "\n";
            return false;
        }

        m_window = SDL_CreateWindow(
            title,
            SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
            w, h,
            SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE
        );
        if (!m_window)
        {
            std::cerr << "SDL_CreateWindow Error: " << SDL_GetError() << "\n";
            SDL_Quit();
            return false;
        }

        SDL_SysWMinfo wminfo{};
        SDL_VERSION(&wminfo.version);
        if (!SDL_GetWindowWMInfo(m_window, &wminfo))
        {
            std::cerr << "SDL_GetWindowWMInfo failed\n";
            return false;
        }

        m_hwnd = wminfo.info.win.window;
        m_width = w;
        m_height = h;
        return true;
    }

    void Application::DestroyWindowSDL()
    {
        if (m_window) { SDL_DestroyWindow(m_window); m_window = nullptr; }
        SDL_Quit();
    }

    void Application::OnResize(int newWidth, int newHeight)
    {
        if (newWidth <= 0 || newHeight <= 0) return; // minimized/invalid
        m_width = newWidth;
        m_height = newHeight;
        m_renderer.OnResize(m_width, m_height);
    }

    // ----------------------
    // Public API
    // ----------------------

    bool Application::Init(int width, int height, const char* title)
    {
        if (m_running) return true;

        // 1) Create window (+ HWND)
        if (!CreateWindowSDL(width, height, title))
            return false;

        // 2) Init renderer with HWND and initial size
        if (!m_renderer.Init(m_hwnd, width, height))
            return false;

        m_running = true;
        return true;
    }

    void Application::Run()
    {
        if (!m_running) return;

        while (m_running)
        {
            // --- Events ---
            SDL_Event e;
            while (SDL_PollEvent(&e))
            {
                switch (e.type)
                {
                case SDL_QUIT:
                    m_running = false;
                    break;
                case SDL_KEYDOWN:
                    if (e.key.keysym.sym == SDLK_ESCAPE)
                        m_running = false;
                    break;
                case SDL_WINDOWEVENT:
                    if (e.window.event == SDL_WINDOWEVENT_SIZE_CHANGED)
                        OnResize(e.window.data1, e.window.data2);
                    break;
                }
            }

            // --- Frame ---
            m_renderer.BeginFrame();
            // TODO: draw calls here
            m_renderer.EndFrame();
        }
    }

    void Application::Shutdown()
    {
        if (!m_running) return;

        m_renderer.Shutdown();
        DestroyWindowSDL();

        m_running = false;
    }
}