#pragma once

#include <SDL2/SDL.h>
#include <cstdint>

struct ID3D11Device;
struct ID3D11DeviceContext;

namespace KibakoEngine {

    namespace DebugUI
    {
        struct RenderStats
        {
            std::uint32_t drawCalls = 0;
            std::uint32_t spritesSubmitted = 0;
        };

        using PanelCallback = void(*)(void* userData);

        void Init(SDL_Window* window, ID3D11Device* device, ID3D11DeviceContext* context);
        void Shutdown();

        void NewFrame();
        void ProcessEvent(const SDL_Event& e);
        void Render();

        void SetEnabled(bool enabled);
        bool IsEnabled();
        void ToggleEnabled();

        void SetVSyncEnabled(bool enabled);
        bool IsVSyncEnabled();

        void SetRenderStats(const RenderStats& stats);
        RenderStats GetRenderStats();

        // Hook pour Scene Inspector (GameLayer l’utilise déjà)
        void SetSceneInspector(void* userData, PanelCallback callback);
    }

} // namespace KibakoEngine