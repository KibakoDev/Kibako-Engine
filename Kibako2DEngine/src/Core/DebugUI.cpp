#include "KibakoEngine/Core/DebugUI.h"

#include "KibakoEngine/Core/Debug.h"
#include "KibakoEngine/Core/Log.h"
#include "KibakoEngine/Core/GameServices.h"

#include <d3d11.h>

// ImGui
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

        RenderStats          g_RenderStats{};
        bool                 g_VSyncEnabled = true;

        void* g_SceneInspectorUserData = nullptr;
        PanelCallback g_SceneInspectorCallback = nullptr;

        constexpr const char* kLogChannel = "DebugUI";
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
        // PAS de docking, ta version d'ImGui ne le supporte pas

        ImGui::StyleColorsDark();

        ImGui_ImplSDL2_InitForD3D(window);
        ImGui_ImplDX11_Init(device, context);

        g_Initialized = true;

        KbkLog(kLogChannel, "Dear ImGui initialized");
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

        KbkLog(kLogChannel, "Dear ImGui shutdown");
    }

    void NewFrame()
    {
        if (!g_Initialized || !g_Enabled)
            return;

        ImGui_ImplDX11_NewFrame();
        ImGui_ImplSDL2_NewFrame();  // ta version ne prend pas de SDL_Window
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

        // ====== Main dock-like window ======
        ImGui::SetNextWindowPos(ImVec2(10.0f, 10.0f), ImGuiCond_Once);
        ImGui::SetNextWindowSize(ImVec2(420.0f, 520.0f), ImGuiCond_Once);

        ImGuiWindowFlags flags =
            ImGuiWindowFlags_NoCollapse |
            ImGuiWindowFlags_NoNav;

        ImGui::Begin("Kibako - Debug", nullptr, flags);

        if (ImGui::BeginTabBar("KibakoDebugTabs"))
        {
            // ================= TAB: ENGINE =================
            if (ImGui::BeginTabItem("Engine"))
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

                // Game time (scaled / raw)
                const GameTime& gt = GameServices::GetTime();

                ImGui::Separator();
                ImGui::Text("Game Time");
                ImGui::BulletText("dt (scaled): %.5f s", gt.scaledDeltaSeconds);
                ImGui::BulletText("dt (raw):    %.5f s", gt.rawDeltaSeconds);
                ImGui::BulletText("Total scaled: %.2f s", gt.totalScaledSeconds);
                ImGui::BulletText("Total raw:    %.2f s", gt.totalRawSeconds);

                float timeScale = static_cast<float>(gt.timeScale);
                if (ImGui::SliderFloat("Time scale", &timeScale, 0.0f, 3.0f, "%.2f")) {
                    GameServices::SetTimeScale(timeScale);
                }

                bool paused = gt.paused;
                if (ImGui::Checkbox("Paused", &paused)) {
                    GameServices::SetPaused(paused);
                }

                ImGui::Separator();
                ImGui::Text("Debug UI");
                ImGui::BulletText("F2: toggle all debug");
                ImGui::BulletText("F1: collision overlay (sandbox)");

                ImGui::EndTabItem();
            }

            // ================= TAB: PERFORMANCE =================
            if (ImGui::BeginTabItem("Performance"))
            {
                static float frameHistory[120] = {};
                static int   historyIndex = 0;
                const int    historySize = IM_ARRAYSIZE(frameHistory);

                float frameMs = (io.Framerate > 0.0f) ? (1000.0f / io.Framerate) : 0.0f;
                frameHistory[historyIndex] = frameMs;
                historyIndex = (historyIndex + 1) % historySize;

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

                ImGui::EndTabItem();
            }

            // ================= TAB: INPUT =================
            if (ImGui::BeginTabItem("Input"))
            {
                ImGui::Text("Mouse");
                ImGui::BulletText("Position: (%.0f, %.0f)", io.MousePos.x, io.MousePos.y);

                ImGui::Separator();
                ImGui::Text("Mouse buttons:");
                ImGui::BulletText("Left:   %s", io.MouseDown[0] ? "Down" : "Up");
                ImGui::BulletText("Right:  %s", io.MouseDown[1] ? "Down" : "Up");
                ImGui::BulletText("Middle: %s", io.MouseDown[2] ? "Down" : "Up");

                ImGui::Separator();
                ImGui::TextDisabled("(Gameplay input géré par KibakoEngine::Input)");

                ImGui::EndTabItem();
            }

            // ================= TAB: SCENE =================
            if (ImGui::BeginTabItem("Scene"))
            {
                if (g_SceneInspectorCallback && g_SceneInspectorUserData) {
                    g_SceneInspectorCallback(g_SceneInspectorUserData);
                }
                else {
                    ImGui::TextDisabled("No Scene2D inspector registered.");
                }

                ImGui::EndTabItem();
            }

            ImGui::EndTabBar();
        }

        ImGui::End(); // "Kibako - Debug"

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