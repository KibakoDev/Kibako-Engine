#pragma once

#include "KibakoEngine/Core/Input.h"
#include "KibakoEngine/Core/Time.h"
#include "KibakoEngine/Renderer/RendererD3D11.h"

struct SDL_Window;
struct HWND__;
using HWND = HWND__*;

namespace KibakoEngine {

    class Application {
    public:
        bool Init(int width, int height, const char* title);
        void Shutdown();

        bool PumpEvents();
        void BeginFrame(const float clearColor[4]);
        void EndFrame(bool waitForVSync = true);

        [[nodiscard]] RendererD3D11& Renderer() { return m_renderer; }
        [[nodiscard]] Input&         InputSys() { return m_input; }
        [[nodiscard]] Time&          TimeSys() { return m_time; }

    private:
        bool CreateWindowSDL(int width, int height, const char* title);
        void DestroyWindowSDL();
        void HandleResize();

    private:
        SDL_Window* m_window = nullptr;
        HWND        m_hwnd = nullptr;
        bool        m_running = false;
        int         m_width = 0;
        int         m_height = 0;

        Time          m_time;
        Input         m_input;
        RendererD3D11 m_renderer;
    };

} // namespace KibakoEngine

