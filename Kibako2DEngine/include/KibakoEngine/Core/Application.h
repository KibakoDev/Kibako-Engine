// Kibako2DEngine/include/KibakoEngine/Core/Application.h
#pragma once

#include <windows.h>

#include "KibakoEngine/Renderer/RendererD3D11.h"
#include "KibakoEngine/Core/Time.h"
#include "KibakoEngine/Core/Input.h"

// Forward-declare to avoid pulling SDL headers in public header
struct SDL_Window;

namespace KibakoEngine
{
    class Application
    {
    public:
        bool Init(int width, int height, const char* title);
        void Run();
        Time  m_time;
        Input m_input;
        void Shutdown();

    private:
        // Window state
        SDL_Window* m_window = nullptr;
        HWND        m_hwnd = nullptr;
        bool        m_running = false;
        int         m_width = 0;
        int         m_height = 0;

        // Rendering backend
        RendererD3D11 m_renderer;

        // Internal helpers
        bool CreateWindowSDL(int w, int h, const char* title);
        void DestroyWindowSDL();
        void OnResize(int newWidth, int newHeight);
    };
}