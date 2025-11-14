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

        // Type générique pour des callbacks de panels custom
        using PanelCallback = void(*)(void* userData);

        // Appelé une seule fois après l'init du renderer
        void Init(SDL_Window* window, ID3D11Device* device, ID3D11DeviceContext* context);

        // Appelé une fois à la fin
        void Shutdown();

        // Une fois par frame, AVANT tout dessin ImGui
        void NewFrame();

        // À appeler pour chaque SDL_Event dans Application::PumpEvents()
        void ProcessEvent(const SDL_Event& e);

        // Après ton rendu 2D (SpriteBatch::End), juste avant Application::EndFrame()
        void Render();

        // Toggle global ON/OFF (F2)
        void SetEnabled(bool enabled);
        bool IsEnabled();
        void ToggleEnabled();

        // Infos renderer (VSync)
        void SetVSyncEnabled(bool enabled);
        bool IsVSyncEnabled();

        // Stats de rendu (SpriteBatch2D)
        void SetRenderStats(const RenderStats& stats);
        RenderStats GetRenderStats();

        // === HOOK SCENE INSPECTOR ===
        // userData  : typiquement un pointeur vers Scene2D (ou ce que tu veux)
        // callback  : fonction qui dessine l'inspecteur ImGui pour cette scène
        void SetSceneInspector(void* userData, PanelCallback callback);
    }

} // namespace KibakoEngine