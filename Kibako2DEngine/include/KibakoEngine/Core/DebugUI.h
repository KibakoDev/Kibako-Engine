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

        // Generic callback signature for custom panels.
        using PanelCallback = void (*)(void* userData);

        // Called once after the renderer has been initialised.
        void Init(SDL_Window* window, ID3D11Device* device, ID3D11DeviceContext* context);

        // Called once during shutdown.
        void Shutdown();

        // Invoke once per frame before any ImGui rendering.
        void NewFrame();

        // Consume every SDL_Event in Application::PumpEvents().
        void ProcessEvent(const SDL_Event& e);

        // Submit ImGui rendering after SpriteBatch::End and before Application::EndFrame.
        void Render();

        // Toggle the debug UI on/off (mapped to F2 in the sandbox).
        void SetEnabled(bool enabled);
        bool IsEnabled();
        void ToggleEnabled();

        // Renderer information (VSync state).
        void SetVSyncEnabled(bool enabled);
        bool IsVSyncEnabled();

        // Sprite batch rendering statistics.
        void SetRenderStats(const RenderStats& stats);
        RenderStats GetRenderStats();

        // === Scene inspector hook ===
        // userData  : typically a pointer to Scene2D (or any context object)
        // callback  : function rendering the ImGui inspector for that scene
        void SetSceneInspector(void* userData, PanelCallback callback);
    }

} // namespace KibakoEngine
