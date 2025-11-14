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

        bool                 g_VSyncEnabled = true;
        RenderStats          g_RenderStats{};

        // Scene inspector callback hook.
        void* g_SceneInspectorUserData = nullptr;
        PanelCallback g_SceneInspectorCallback = nullptr;
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

        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
        // Docking is not enabled for the current ImGui configuration.

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

        g_SceneInspectorUserData = nullptr;
        g_SceneInspectorCallback = nullptr;

        KbkLog("DebugUI", "Dear ImGui shutdown");
    }

    void NewFrame()
    {
        if (!g_Initialized || !g_Enabled)
            return;

        ImGui_ImplDX11_NewFrame();
        ImGui_ImplSDL2_NewFrame();
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

        ImGuiIO& io = ImGui::GetIO();

        // ==========================
        // 1) ENGINE PANEL
        // ==========================
        ImGui::Begin("Kibako - Engine");
        {
            const float fps = io.Framerate;
            const float frameMs = (fps > 0.0f) ? (1000.0f / fps) : 0.0f;
            const float deltaTime = io.DeltaTime;
            const ImVec2 display = io.DisplaySize;

            ImGui::Text("Frame: %.3f ms (%.1f FPS)", frameMs, fps);
            ImGui::Text("DeltaTime: %.5f s", deltaTime);

            ImGui::Separator();
            ImGui::Text("Renderer");
            ImGui::BulletText("Backbuffer: %.0f x %.0f", display.x, display.y);
            ImGui::BulletText("VSync: %s", g_VSyncEnabled ? "ON" : "OFF");
            ImGui::BulletText("Sprites: %u", g_RenderStats.spritesSubmitted);
            ImGui::BulletText("Draw calls: %u", g_RenderStats.drawCalls);

            ImGui::Separator();
            ImGui::Text("Debug UI: %s", g_Enabled ? "ENABLED" : "DISABLED");
            ImGui::Text("F2: toggle ImGui");
            ImGui::Text("F1: collision overlay");
        }
        ImGui::End();

        // ==========================
        // 2) PERFORMANCE PANEL
        // ==========================
        {
            static float frameHistory[120] = {};
            static int   historyIndex = 0;
            const int    historySize = IM_ARRAYSIZE(frameHistory);

            float frameMs = (io.Framerate > 0.0f) ? (1000.0f / io.Framerate) : 0.0f;
            frameHistory[historyIndex] = frameMs;
            historyIndex = (historyIndex + 1) % historySize;

            ImGui::Begin("Kibako - Performance");
            ImGui::Text("Frame time history (ms)");
            ImGui::PlotLines(
                "##frametime",
                frameHistory,
                historySize,
                historyIndex,
                nullptr,
                0.0f,
                40.0f,
                ImVec2(0.0f, 80.0f)
            );
            ImGui::Text("Target: 16.6 ms (60 FPS)");
            ImGui::End();
        }

        // ==========================
        // 3) INPUT PANEL
        // ==========================
        ImGui::Begin("Kibako - Input");
        {
            ImGui::Text("Mouse position: (%.0f, %.0f)", io.MousePos.x, io.MousePos.y);

            ImGui::Separator();
            ImGui::Text("Mouse buttons:");
            ImGui::BulletText("Left:   %s", io.MouseDown[0] ? "Down" : "Up");
            ImGui::BulletText("Right:  %s", io.MouseDown[1] ? "Down" : "Up");
            ImGui::BulletText("Middle: %s", io.MouseDown[2] ? "Down" : "Up");

            ImGui::Separator();
            ImGui::TextDisabled("(Gameplay input is handled by the engine; this is the ImGui view.)");
        }
        ImGui::End();

        // ==========================
        // 4) SCENE INSPECTOR PANEL
        // ==========================
        if (g_SceneInspectorCallback && g_SceneInspectorUserData) {
            g_SceneInspectorCallback(g_SceneInspectorUserData);
        }

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

    void SetVSyncEnabled(bool enabled)
    {
        g_VSyncEnabled = enabled;
    }

    bool IsVSyncEnabled()
    {
        return g_VSyncEnabled;
    }

    void SetRenderStats(const RenderStats& stats)
    {
        g_RenderStats = stats;
    }

    RenderStats GetRenderStats()
    {
        return g_RenderStats;
    }

    void SetSceneInspector(void* userData, PanelCallback callback)
    {
        g_SceneInspectorUserData = userData;
        g_SceneInspectorCallback = callback;
    }

} // namespace KibakoEngine::DebugUI
