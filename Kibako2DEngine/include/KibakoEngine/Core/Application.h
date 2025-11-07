// =====================================================
// Kibako2DEngine/Core/Application.h
// App shell: SDL window, event pump, timing, input,
// and ownership of the D3D11 renderer.
// Debug-ready with logging and asserts.
// =====================================================

#pragma once
#include <windows.h>

#include "KibakoEngine/Core/Time.h"
#include "KibakoEngine/Core/Input.h"
#include "KibakoEngine/Renderer/RendererD3D11.h"

// Forward declare to avoid pulling SDL headers here
struct SDL_Window;

namespace KibakoEngine {

    class Application {
    public:
        // Create SDL window and initialize renderer
        bool Init(int width, int height, const char* title);

        // Drive one frame of OS events; returns false to quit
        bool PumpEvents();

        // Begin/End GPU frame
        void BeginFrame();
        void EndFrame();

        // Shutdown and free resources
        void Shutdown();

        // Accessors used by Sandbox
        RendererD3D11& Renderer() { return m_renderer; }
        Time& TimeSys() { return m_time; }
        Input& InputSys() { return m_input; }

    private:
        // Window
        bool CreateWindowSDL(int w, int h, const char* title);
        void DestroyWindowSDL();
        void OnResize(int newWidth, int newHeight);

    private:
        // OS windowing
        SDL_Window* m_window = nullptr;
        HWND        m_hwnd = nullptr;
        bool        m_running = false;
        int         m_width = 0;
        int         m_height = 0;

        // Subsystems
        Time         m_time;
        Input        m_input;
        RendererD3D11 m_renderer;
    };

}