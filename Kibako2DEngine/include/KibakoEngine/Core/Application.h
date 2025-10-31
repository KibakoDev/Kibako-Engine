// =====================================================
// Kibako2DEngine/Core/Application.h
// Main application layer: handles window, input, timing,
// and owns the renderer.
// =====================================================

#pragma once
#include <windows.h>

#include "KibakoEngine/Renderer/RendererD3D11.h"
#include "KibakoEngine/Core/Time.h"
#include "KibakoEngine/Core/Input.h"
#include "KibakoEngine/Renderer/Texture2D.h"
#include "KibakoEngine/Renderer/SpriteTypes.h"

// Forward declare SDL_Window to avoid including SDL headers
struct SDL_Window;

namespace KibakoEngine {

    class Application {
    public:
        // Initializes SDL, creates a window, and sets up D3D11
        bool Init(int width, int height, const char* title);

        // Main application loop (runs until quit)
        void Run();

        // Gracefully shuts down the app and releases resources
        void Shutdown();

        // Exposed subsystems (useful for debugging)
        Time  m_time;   // Handles delta time and FPS
        Input m_input;  // Handles keyboard / mouse input

    private:
        // --- Window state ---
        SDL_Window* m_window = nullptr;
        HWND        m_hwnd = nullptr;
        bool        m_running = false;
        int         m_width = 0;
        int         m_height = 0;

        // --- Core systems ---
        RendererD3D11 m_renderer;  // Direct3D11 backend
        Texture2D     m_debugTexture; // Temporary test texture

        // --- Internal helpers ---
        bool CreateWindowSDL(int w, int h, const char* title);
        void DestroyWindowSDL();
        void OnResize(int newWidth, int newHeight);
    };

}