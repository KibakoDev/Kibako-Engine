// =====================================================
// KibakoEngine/Core/Application.h
// Main engine application:
// - Owns SDL window + Win32 HWND
// - Owns D3D11 renderer, input, time
// - Drives a simple Layer stack
// =====================================================

#pragma once

#include <cstdint>
#include <vector>
#include <windows.h>

#include "KibakoEngine/Core/Time.h"
#include "KibakoEngine/Core/Input.h"
#include "KibakoEngine/Renderer/RendererD3D11.h"

struct SDL_Window;

namespace KibakoEngine {

    class Layer; // forward

    class Application
    {
    public:
        Application() = default;
        ~Application() = default;

        // Create SDL window + D3D11 renderer
        bool Init(int width, int height, const char* title);
        void Shutdown();

        // --- Two ways to run the app ---

        // 1) "Manual" loop (what you're doing now in main)
        bool PumpEvents();                         // input + time + SDL events
        void BeginFrame(const float clearColor[4]);
        void EndFrame(bool waitForVSync = true);

        // 2) Engine-driven loop using Layers
        //    (optional, if you want Application to own the main loop)
        void Run(const float clearColor[4], bool waitForVSync = true);

        // --- Accessors ---
        RendererD3D11& Renderer() { return m_renderer; }
        const RendererD3D11& Renderer() const { return m_renderer; }

        Time& TimeSys() { return m_time; }
        const Time& TimeSys() const { return m_time; }

        Input& InputSys() { return m_input; }
        const Input& InputSys() const { return m_input; }

        int  Width()  const { return m_width; }
        int  Height() const { return m_height; }

        // --- Layer stack ---
        void PushLayer(Layer* layer);
        void PopLayer(Layer* layer);

    private:
        bool CreateWindowSDL(int width, int height, const char* title);
        void DestroyWindowSDL();
        void HandleResize();

    private:
        // Window
        SDL_Window* m_window = nullptr;
        HWND        m_hwnd = nullptr;

        int  m_width = 0;
        int  m_height = 0;
        bool m_running = false;

        // Core subsystems
        RendererD3D11 m_renderer;
        Time          m_time;
        Input         m_input;

        // Layer stack (not owning – Sandbox owns Layers)
        std::vector<Layer*> m_layers;
    };

} // namespace KibakoEngine