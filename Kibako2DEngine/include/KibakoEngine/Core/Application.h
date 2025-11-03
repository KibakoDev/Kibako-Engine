// =====================================================
// Kibako2DEngine/Core/Application.h
// Core application layer: creates window, handles input,
// timing, and connects to the Direct3D11 renderer.
// The sandbox owns the main loop logic.
// =====================================================

#pragma once
#include <windows.h>

#include "KibakoEngine/Core/Time.h"
#include "KibakoEngine/Core/Input.h"
#include "KibakoEngine/Renderer/RendererD3D11.h"

// Forward-declare SDL types to avoid including full SDL headers here
struct SDL_Window;

namespace KibakoEngine {

    class Application {
    public:
        // ----------------------------------------------
        // Lifecycle
        // ----------------------------------------------
        bool Init(int width, int height, const char* title);
        bool PumpEvents();   // Polls SDL events (returns false to quit)
        void BeginFrame();   // Starts frame (clears target)
        void EndFrame();     // Presents frame and updates timing
        void Shutdown();     // Cleanly releases everything

        // ----------------------------------------------
        // Subsystem accessors (used by sandbox)
        // ----------------------------------------------
        RendererD3D11& Renderer() { return m_renderer; }
        Time& TimeSys() { return m_time; }
        Input& InputSys() { return m_input; }

    private:
        // ----------------------------------------------
        // SDL Window management
        // ----------------------------------------------
        bool CreateWindowSDL(int width, int height, const char* title);
        void DestroyWindowSDL();
        void OnResize(int newWidth, int newHeight);

        // ----------------------------------------------
        // Internal state
        // ----------------------------------------------
        SDL_Window* m_window = nullptr;
        HWND        m_hwnd = nullptr;
        bool        m_running = false;

        int m_width = 0;
        int m_height = 0;

        // Core systems
        RendererD3D11 m_renderer;
        Time          m_time;
        Input         m_input;
    };
}