// DebugUI.h - Declares the Dear ImGui-based debug overlay utilities.
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

        using PanelCallback = void (*)(void* userData);

        void Init(SDL_Window* window, ID3D11Device* device, ID3D11DeviceContext* context);
        void Shutdown();

        void NewFrame();
        void ProcessEvent(const SDL_Event& e);
        void Render();

        void SetEnabled(bool enabled);
        [[nodiscard]] bool IsEnabled();
        void ToggleEnabled();

        void SetVSyncEnabled(bool enabled);
        [[nodiscard]] bool IsVSyncEnabled();

        void SetRenderStats(const RenderStats& stats);
        [[nodiscard]] RenderStats GetRenderStats();

        // Registers an external scene inspector panel (implemented by the sandbox).
        void SetSceneInspector(void* userData, PanelCallback callback);
    }

} // namespace KibakoEngine
