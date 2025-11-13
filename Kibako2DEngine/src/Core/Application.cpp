// =====================================================
// KibakoEngine/Core/Application.cpp
// SDL window + event pump + D3D11 renderer + Layer stack
// =====================================================

#include "KibakoEngine/Core/Application.h"

#include "KibakoEngine/Core/Debug.h"
#include "KibakoEngine/Core/Log.h"
#include "KibakoEngine/Core/Layer.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_syswm.h>

#include <algorithm> // std::find

namespace KibakoEngine {

    // -------------------------------------------------
    // Window creation / destruction
    // -------------------------------------------------
    bool Application::CreateWindowSDL(int width, int height, const char* title)
    {
        SDL_SetMainReady();
        if (SDL_Init(SDL_INIT_VIDEO) != 0) {
            KbkLog("App", "SDL_Init failed: %s", SDL_GetError());
            return false;
        }

        m_window = SDL_CreateWindow(
            title,
            SDL_WINDOWPOS_CENTERED,
            SDL_WINDOWPOS_CENTERED,
            width,
            height,
            SDL_WINDOW_RESIZABLE | SDL_WINDOW_SHOWN
        );
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

    // -------------------------------------------------
    // Resize handling
    // -------------------------------------------------
    void Application::HandleResize()
    {
        if (!m_window)
            return;

        int drawableW = 0;
        int drawableH = 0;
        SDL_GetWindowSizeInPixels(m_window, &drawableW, &drawableH);
        if (drawableW <= 0 || drawableH <= 0)
            return; // minimized, etc.

        if (drawableW == m_width && drawableH == m_height)
            return; // nothing changed

        m_width = drawableW;
        m_height = drawableH;

        KbkLog("App", "Resize -> %dx%d", m_width, m_height);

        m_renderer.OnResize(
            static_cast<uint32_t>(m_width),
            static_cast<uint32_t>(m_height)
        );
    }

    // -------------------------------------------------
    // Init / Shutdown
    // -------------------------------------------------
    bool Application::Init(int width, int height, const char* title)
    {
        if (m_running)
            return true;

        // SDL window + HWND
        if (!CreateWindowSDL(width, height, title))
            return false;

        SDL_GetWindowSizeInPixels(m_window, &m_width, &m_height);
        KbkLog("App", "Drawable size: %dx%d", m_width, m_height);

        // Renderer
        if (!m_renderer.Init(
            m_hwnd,
            static_cast<uint32_t>(m_width),
            static_cast<uint32_t>(m_height))) {
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

        // Detach all layers (Sandbox owns them)
        for (Layer* layer : m_layers) {
            if (layer)
                layer->OnDetach();
        }
        m_layers.clear();

        m_renderer.Shutdown();
        DestroyWindowSDL();

        m_running = false;
    }

    // -------------------------------------------------
    // Event pump (used both by manual loop and Run())
    // -------------------------------------------------
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
                switch (evt.window.event) {
                case SDL_WINDOWEVENT_SIZE_CHANGED:
                case SDL_WINDOWEVENT_RESIZED:
                case SDL_WINDOWEVENT_MAXIMIZED:
                case SDL_WINDOWEVENT_RESTORED:
#ifdef SDL_WINDOWEVENT_DISPLAY_SCALE_CHANGED
                case SDL_WINDOWEVENT_DISPLAY_SCALE_CHANGED:
#endif
                    HandleResize();
                    break;
                default:
                    break;
                }
                break;

            case SDL_KEYDOWN:
                if (evt.key.keysym.scancode == SDL_SCANCODE_ESCAPE)
                    return false;
                break;

            default:
                break;
            }

            // Forward to input system
            m_input.HandleEvent(evt);
        }

        return true;
    }

    // -------------------------------------------------
    // Frame control
    // -------------------------------------------------
    void Application::BeginFrame(const float clearColor[4])
    {
        m_renderer.BeginFrame(clearColor);
    }

    void Application::EndFrame(bool waitForVSync)
    {
        m_renderer.EndFrame(waitForVSync);
        m_input.EndFrame();
    }

    // -------------------------------------------------
    // Engine-driven main loop using Layers
    // -------------------------------------------------
    void Application::Run(const float clearColor[4], bool waitForVSync)
    {
        KBK_ASSERT(m_running, "Run() called before Init()");

        while (PumpEvents()) {
            // Layer API uses float dt, so we convert explicitly
            const float dt = static_cast<float>(m_time.DeltaSeconds());

            // Update all layers
            for (Layer* layer : m_layers) {
                if (layer)
                    layer->OnUpdate(dt);
            }

            // Render all layers with one shared sprite batch
            BeginFrame(clearColor);

            auto& batch = m_renderer.Batch();
            batch.Begin(m_renderer.Camera().GetViewProjectionT());

            for (Layer* layer : m_layers) {
                if (layer)
                    layer->OnRender(batch);
            }

            batch.End();
            EndFrame(waitForVSync);
        }
    }

    // -------------------------------------------------
    // Layer stack
    // -------------------------------------------------
    void Application::PushLayer(Layer* layer)
    {
        if (!layer)
            return;

        m_layers.push_back(layer);
        layer->OnAttach();
    }

    void Application::PopLayer(Layer* layer)
    {
        if (!layer)
            return;

        auto it = std::find(m_layers.begin(), m_layers.end(), layer);
        if (it != m_layers.end()) {
            (*it)->OnDetach();
            m_layers.erase(it);
        }
    }

} // namespace KibakoEngine