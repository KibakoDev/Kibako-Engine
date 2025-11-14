#include "KibakoEngine/Core/DebugUI.h"

#include "KibakoEngine/Core/Debug.h"
#include "KibakoEngine/Core/Log.h"

#include <d3d11.h>

// Dear ImGui
#include "imgui.h"
#include "backends/imgui_impl_dx11.h"
#include "backends/imgui_impl_sdl2.h"

namespace KibakoEngine::DebugUI {

    namespace
    {
        bool                 g_Enabled = true;
        bool                 g_Initialized = false;
        ID3D11Device* g_Device = nullptr;
        ID3D11DeviceContext* g_Context = nullptr;
    }

    void Init(SDL_Window* window, ID3D11Device* device, ID3D11DeviceContext* context)
    {
        KBK_ASSERT(!g_Initialized, "DebugUI::Init called twice");
        KBK_ASSERT(window != nullptr, "DebugUI::Init: window is null");
        KBK_ASSERT(device != nullptr && context != nullptr, "DebugUI::Init: device/context null");

        g_Device = device;
        g_Context = context;

        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();
        (void)io;

        // Si ta version ne supporte pas Docking, on ne le touche pas.
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
        // io.ConfigFlags |= ImGuiConfigFlags_DockingEnable; // désactivé pour compat

        ImGui::StyleColorsDark();

        ImGui_ImplSDL2_InitForD3D(window);
        ImGui_ImplDX11_Init(device, context);

        g_Initialized = true;

        KbkLog("DebugUI", "Dear ImGui initialized");
    }

    void Shutdown()
    {
        if (!g_Initialized)
            return;

        ImGui_ImplDX11_Shutdown();
        ImGui_ImplSDL2_Shutdown();
        ImGui::DestroyContext();

        g_Device = nullptr;
        g_Context = nullptr;
        g_Initialized = false;

        KbkLog("DebugUI", "Dear ImGui shutdown");
    }

    void NewFrame()
    {
        if (!g_Initialized || !g_Enabled)
            return;

        ImGui_ImplDX11_NewFrame();
        ImGui_ImplSDL2_NewFrame();   // <-- plus de paramètre
        ImGui::NewFrame();
    }

    void ProcessEvent(const SDL_Event& e)
    {
        if (!g_Initialized || !g_Enabled)
            return;

        ImGui_ImplSDL2_ProcessEvent(&e);
    }

    void Render()
    {
        if (!g_Initialized || !g_Enabled)
            return;

        ImGui::Begin("Kibako Debug");
        {
            ImGuiIO& io = ImGui::GetIO();
            ImGui::Text("FPS: %.1f", io.Framerate);
            ImGui::Separator();
            ImGui::Text("Debug UI: %s", g_Enabled ? "ENABLED" : "DISABLED");
            ImGui::Text("F2: toggle ImGui");
            ImGui::Text("F1: collision overlay");
        }
        ImGui::End();

        // ImGui::ShowDemoWindow(); // active ça si tu veux explorer

        ImGui::Render();
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
    }

    void SetEnabled(bool enabled)
    {
        g_Enabled = enabled;
    }

    bool IsEnabled()
    {
        return g_Enabled;
    }

    void ToggleEnabled()
    {
        g_Enabled = !g_Enabled;
    }

} // namespace KibakoEngine::DebugUI