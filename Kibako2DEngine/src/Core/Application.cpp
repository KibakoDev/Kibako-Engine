// =====================================================
// Kibako2DEngine/Core/Application.cpp
// Main application entry: manages window, input, timing,
// and the main loop using SDL + D3D11.
// =====================================================

#define WIN32_LEAN_AND_MEAN
#define SDL_MAIN_HANDLED

#include <windows.h>
#include <iostream>

#include <SDL2/SDL.h>
#include <SDL2/SDL_syswm.h>

#include "KibakoEngine/Core/Application.h"
#include "KibakoEngine/Renderer/Texture2D.h"

namespace KibakoEngine {

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

        // Create a resizable window
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

        // Extract HWND for Direct3D
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
        int pxW = 0, pxH = 0;
        SDL_GetWindowSizeInPixels(m_window, &pxW, &pxH);
        if (pxW <= 0 || pxH <= 0) return; // minimized / invalid size

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

        // 1) Create SDL window
        if (!CreateWindowSDL(width, height, title))
            return false;

        // 2) Get true pixel size (handles HiDPI)
        SDL_GetWindowSizeInPixels(m_window, &m_width, &m_height);

        // 3) Initialize D3D11 renderer
        if (!m_renderer.Init(m_hwnd, m_width, m_height))
            return false;

        // 4) Load test texture
        static Texture2D tex;
        static bool loaded = false;
        if (!loaded)
        {
            loaded = tex.LoadFromFile(m_renderer.GetDevice(), "assets/star.png", true);
            if (loaded)
                std::cout << "Loaded texture: " << tex.Width() << "x" << tex.Height() << "\n";
            else
                std::cerr << "Failed to load texture\n";
        }

        m_debugTexture = tex; // store reference to test texture
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

            // ---------------------------
            // Handle SDL events
            // ---------------------------
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
                    switch (e.window.event)
                    {
                    case SDL_WINDOWEVENT_SIZE_CHANGED:
                    case SDL_WINDOWEVENT_RESIZED:
                        OnResize(e.window.data1, e.window.data2);
                        break;

#if defined(SDL_WINDOWEVENT_DISPLAY_SCALE_CHANGED)
                    case SDL_WINDOWEVENT_DISPLAY_SCALE_CHANGED:
                        OnResize(0, 0); // re-fetch real pixel size
                        break;
#endif
                    default:
                        break;
                    }
                    break;

                default:
                    break;
                }

                m_input.HandleEvent(e);
            }

            // ---------------------------
            // CAMERA CONTROL
            // ---------------------------
            {
                float dt = static_cast<float>(m_time.DeltaSeconds());
                auto& cam = m_renderer.Camera();

                const float baseMove = 600.0f;
                const float move = baseMove * dt / cam.Zoom();

                const float zoomStep = 1.5f * dt;
                const float rotSpeed = 1.5f * dt;

                // Move with WASD
                if (m_input.KeyDown(SDL_SCANCODE_W)) cam.Move(0.0f, -move);
                if (m_input.KeyDown(SDL_SCANCODE_S)) cam.Move(0.0f, move);
                if (m_input.KeyDown(SDL_SCANCODE_A)) cam.Move(-move, 0.0f);
                if (m_input.KeyDown(SDL_SCANCODE_D)) cam.Move(move, 0.0f);

                // Rotate with Q/E
                if (m_input.KeyDown(SDL_SCANCODE_Q)) cam.AddRotation(-rotSpeed);
                if (m_input.KeyDown(SDL_SCANCODE_E)) cam.AddRotation(rotSpeed);

                // Zoom with Z/X or mouse wheel
                if (m_input.KeyDown(SDL_SCANCODE_Z)) cam.AddZoom(zoomStep);
                if (m_input.KeyDown(SDL_SCANCODE_X)) cam.AddZoom(-zoomStep);
                if (m_input.WheelY() != 0)
                    cam.AddZoom((float)m_input.WheelY() * -0.10f);

                // Reset with R
                if (m_input.KeyDown(SDL_SCANCODE_R)) cam.Reset();
            }

            // =====================================================
            // RENDER
            // =====================================================
            m_renderer.BeginFrame();

            auto& sprites = m_renderer.Sprites();
            sprites.Begin(m_renderer.Camera().GetViewProjT());

            if (m_debugTexture.GetSRV())
            {
                // Destination rect in world space
                RectF dst{ 200.0f, 150.0f, (float)m_debugTexture.Width(), (float)m_debugTexture.Height() };
                RectF src{ 0.0f, 0.0f, 1.0f, 1.0f };
                Color4 tint = Color4::White();

                sprites.SetMonochrome(0.0f); // 0 = color, 1 = full grayscale
                sprites.DrawSprite(m_debugTexture, dst, src, tint, 0.0f);
            }

            sprites.End();
            m_renderer.EndFrame();

            m_input.EndFrame();

            // Print FPS every ~0.5s
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