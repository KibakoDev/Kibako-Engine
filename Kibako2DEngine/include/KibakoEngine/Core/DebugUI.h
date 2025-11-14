#pragma once

#include <SDL2/SDL.h>

struct ID3D11Device;
struct ID3D11DeviceContext;

namespace KibakoEngine {

    namespace DebugUI
    {
        // Appelé une seule fois après l'init du renderer
        void Init(SDL_Window* window, ID3D11Device* device, ID3D11DeviceContext* context);

        // Appelé une fois à la fin
        void Shutdown();

        // Une fois par frame, AVANT tout dessin ImGui
        void NewFrame();  // <-- plus de paramètre

        // À appeler pour chaque SDL_Event dans Application::PumpEvents()
        void ProcessEvent(const SDL_Event& e);

        // Après ton rendu 2D (SpriteBatch::End), juste avant Application::EndFrame()
        void Render();

        // Toggle global ON/OFF (F2)
        void SetEnabled(bool enabled);
        bool IsEnabled();
        void ToggleEnabled();
    }

} // namespace KibakoEngine