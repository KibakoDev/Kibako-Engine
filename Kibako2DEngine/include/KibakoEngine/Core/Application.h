#pragma once

#include <cstdint>
#include <vector>

#include "KibakoEngine/Core/Input.h"
#include "KibakoEngine/Core/Time.h"
#include "KibakoEngine/Renderer/RendererD3D11.h"

struct SDL_Window;
struct HWND__;
using HWND = HWND__*;

namespace KibakoEngine {

    class Layer;

    class Application
    {
    public:
        Application() = default;
        ~Application() = default;

        bool Init(int width, int height, const char* title);
        void Shutdown();

        bool PumpEvents();
        void BeginFrame(const float clearColor[4]);
        void EndFrame(bool waitForVSync = true);
        void Run(const float clearColor[4], bool waitForVSync = true);

        RendererD3D11& Renderer() { return m_renderer; }
        const RendererD3D11& Renderer() const { return m_renderer; }

        Time& TimeSys() { return m_time; }
        const Time& TimeSys() const { return m_time; }

        Input& InputSys() { return m_input; }
        const Input& InputSys() const { return m_input; }

        int Width() const { return m_width; }
        int Height() const { return m_height; }

        void PushLayer(Layer* layer);
        void PopLayer(Layer* layer);

    private:
        bool CreateWindowSDL(int width, int height, const char* title);
        void DestroyWindowSDL();
        void HandleResize();

        SDL_Window* m_window = nullptr;
        HWND        m_hwnd = nullptr;

        int  m_width = 0;
        int  m_height = 0;
        bool m_running = false;

        RendererD3D11 m_renderer;
        Time          m_time;
        Input         m_input;

        std::vector<Layer*> m_layers;
    };

} // namespace KibakoEngine

