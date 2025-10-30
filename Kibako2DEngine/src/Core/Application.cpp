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
    // =====================================================
    // WINDOW CREATION / MANAGEMENT
    // =====================================================
    bool Application::CreateWindowSDL(int w, int h, const char* title)
    {
        SDL_SetMainReady();
        if (SDL_Init(SDL_INIT_VIDEO) != 0)
        {
            std::cerr << "SDL_Init Error: " << SDL_GetError() << "\n";
            return false;
        }

        // Create a resizable SDL window
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

        // Extract HWND from SDL (for Direct3D)
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
        if (m_window)
        {
            SDL_DestroyWindow(m_window);
            m_window = nullptr;
        }
        SDL_Quit();
    }

    // =====================================================
    // HANDLE WINDOW RESIZE
    // =====================================================
    void Application::OnResize(int /*newWidth*/, int /*newHeight*/)
    {
        // Query true drawable pixel size (handles DPI scaling)
        int pxW = 0, pxH = 0;
        SDL_GetWindowSizeInPixels(m_window, &pxW, &pxH);
        if (pxW <= 0 || pxH <= 0) return; // minimized / invalid

        m_width = pxW;
        m_height = pxH;

        // Notify renderer
        m_renderer.OnResize(m_width, m_height);
    }

    // =====================================================
    // INITIALIZATION
    // =====================================================
    bool Application::Init(int width, int height, const char* title)
    {
        if (m_running) return true;

        // Create SDL window
        if (!CreateWindowSDL(width, height, title))
            return false;

        // Get actual pixel size (handles DPI)
        int pxW = 0, pxH = 0;
        SDL_GetWindowSizeInPixels(m_window, &pxW, &pxH);
        m_width = pxW;
        m_height = pxH;

        // Initialize D3D11 renderer
        if (!m_renderer.Init(m_hwnd, m_width, m_height))
            return false;

        m_running = true;
        return true;
    }

    // =====================================================
    // MAIN LOOP
    // =====================================================
    void Application::Run()
    {
        if (!m_running) return;

        while (m_running)
        {
            m_input.BeginFrame();
            m_time.Tick();

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
                {
                    switch (e.window.event)
                    {
                    case SDL_WINDOWEVENT_SIZE_CHANGED:
                    case SDL_WINDOWEVENT_RESIZED:
                        OnResize(e.window.data1, e.window.data2);
                        break;

#if defined(SDL_WINDOWEVENT_DISPLAY_SCALE_CHANGED)
                    case SDL_WINDOWEVENT_DISPLAY_SCALE_CHANGED:
                        OnResize(0, 0); // re-read pixel size
                        break;
#endif
                    default:
                        break;
                    }
                }
                break;

                default:
                    break;
                }

                // Pass event to input system
                m_input.HandleEvent(e);
            }

            // --- Camera controls (per-frame) ---
            {
                float dt = static_cast<float>(m_time.DeltaSeconds());
                auto& cam = m_renderer.Camera();

                // Move speed in pixels/sec, adjusted by zoom so movement feels consistent
                const float baseMove = 600.0f;
                const float move = baseMove * dt / cam.Zoom();

                // Zoom and rotation speed
                const float zoomStep = 1.5f * dt;     // add/sub zoom per second
                const float rotSpeed = 1.5f * dt;     // radians per second

                // WASD: pan
                if (m_input.KeyDown(SDL_SCANCODE_W)) cam.Move(0.0f, -move);
                if (m_input.KeyDown(SDL_SCANCODE_S)) cam.Move(0.0f, move);
                if (m_input.KeyDown(SDL_SCANCODE_A)) cam.Move(-move, 0.0f);
                if (m_input.KeyDown(SDL_SCANCODE_D)) cam.Move(move, 0.0f);

                // QE: rotate
                if (m_input.KeyDown(SDL_SCANCODE_Q)) cam.AddRotation(-rotSpeed);
                if (m_input.KeyDown(SDL_SCANCODE_E)) cam.AddRotation(rotSpeed);

                // ZX: zoom
                if (m_input.KeyDown(SDL_SCANCODE_Z)) cam.AddZoom(zoomStep);
                if (m_input.KeyDown(SDL_SCANCODE_X)) cam.AddZoom(-zoomStep);

                // Mouse wheel: zoom (optional, smooth)
                if (m_input.WheelY() != 0) {
                    cam.AddZoom((float)m_input.WheelY() * 0.10f); // tweak factor if needed
                }

                // R: reset camera
                if (m_input.KeyDown(SDL_SCANCODE_R)) cam.Reset();
            }

            // Render one frame
            m_renderer.BeginFrame();
            // TODO: update & draw using dt / input
            m_renderer.EndFrame();

            m_input.EndFrame();

            // Simple FPS print (every ~0.5s)
            static double acc = 0.0;
            acc += m_time.DeltaSeconds();
            if (acc > 0.5)
            {
                std::cout << "FPS: " << (int)m_time.FPS() << "\r";
                acc = 0.0;
            }
        }
    }

    // =====================================================
    // SHUTDOWN
    // =====================================================
    void Application::Shutdown()
    {
        if (!m_running) return;

        m_renderer.Shutdown();
        DestroyWindowSDL();

        m_running = false;
    }
}