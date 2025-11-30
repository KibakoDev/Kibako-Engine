// Debug UI utilities
#pragma once

#include <SDL2/SDL.h>
#include <cstdint>

#include "KibakoEngine/Core/Debug.h"

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

#if KBK_DEBUG_BUILD
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

        // Register an external scene inspector panel
        void SetSceneInspector(void* userData, PanelCallback callback);
#else
        inline void Init(SDL_Window* window, ID3D11Device* device, ID3D11DeviceContext* context)
        {
            KBK_UNUSED(window);
            KBK_UNUSED(device);
            KBK_UNUSED(context);
        }

        inline void Shutdown() {}

        inline void NewFrame() {}

        inline void ProcessEvent(const SDL_Event& e)
        {
            KBK_UNUSED(e);
        }

        inline void Render() {}

        inline void SetEnabled(bool enabled)
        {
            KBK_UNUSED(enabled);
        }

        [[nodiscard]] inline bool IsEnabled()
        {
            return false;
        }

        inline void ToggleEnabled() {}

        inline void SetVSyncEnabled(bool enabled)
        {
            KBK_UNUSED(enabled);
        }

        [[nodiscard]] inline bool IsVSyncEnabled()
        {
            return false;
        }

        inline void SetRenderStats(const RenderStats& stats)
        {
            KBK_UNUSED(stats);
        }

        [[nodiscard]] inline RenderStats GetRenderStats()
        {
            return {};
        }

        inline void SetSceneInspector(void* userData, PanelCallback callback)
        {
            KBK_UNUSED(userData);
            KBK_UNUSED(callback);
        }
#endif
    }

} // namespace KibakoEngine
