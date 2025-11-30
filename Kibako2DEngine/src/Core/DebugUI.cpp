// Debug UI overlay
#include "KibakoEngine/Core/DebugUI.h"

#if KBK_DEBUG_BUILD

#include "KibakoEngine/Core/Debug.h"
#include "KibakoEngine/Core/GameServices.h"
#include "KibakoEngine/Core/Log.h"

#include <d3d11.h>

#include "imgui.h"
#include "backends/imgui_impl_dx11.h"
#include "backends/imgui_impl_sdl2.h"

namespace KibakoEngine::DebugUI {

    namespace
    {
        constexpr const char* kLogChannel = "DebugUI";

        bool                 g_enabled = true;
        bool                 g_initialized = false;
        ID3D11Device*        g_device = nullptr;
        ID3D11DeviceContext* g_context = nullptr;

        RenderStats g_renderStats{};
        bool        g_vsyncEnabled = true;

        void*         g_sceneInspectorUserData = nullptr;
        PanelCallback g_sceneInspectorCallback = nullptr;
    }

    void Init(SDL_Window* window, ID3D11Device* device, ID3D11DeviceContext* context)
    {
        KBK_ASSERT(!g_initialized, "DebugUI::Init called twice");
        KBK_ASSERT(window != nullptr, "DebugUI::Init requires a valid SDL window");
        KBK_ASSERT(device != nullptr && context != nullptr, "DebugUI::Init requires valid D3D11 handles");

        g_device = device;
        g_context = context;

        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

        ImGui::StyleColorsDark();

        ImGui_ImplSDL2_InitForD3D(window);
        ImGui_ImplDX11_Init(device, context);

        g_initialized = true;

        KbkLog(kLogChannel, "Dear ImGui initialized");
    }

    void Shutdown()
    {
        if (!g_initialized)
            return;

        ImGui_ImplDX11_Shutdown();
        ImGui_ImplSDL2_Shutdown();
        ImGui::DestroyContext();

        g_device = nullptr;
        g_context = nullptr;
        g_initialized = false;

        g_sceneInspectorUserData = nullptr;
        g_sceneInspectorCallback = nullptr;

        KbkLog(kLogChannel, "Dear ImGui shutdown");
    }

    void NewFrame()
    {
        if (!g_initialized || !g_enabled)
            return;

        ImGui_ImplDX11_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();
    }

    void ProcessEvent(const SDL_Event& e)
    {
        if (!g_initialized || !g_enabled)
            return;

        ImGui_ImplSDL2_ProcessEvent(&e);
    }

    void Render()
    {
        if (!g_initialized || !g_enabled)
            return;

        ImGuiIO& io = ImGui::GetIO();

        ImGui::SetNextWindowPos(ImVec2(10.0f, 10.0f), ImGuiCond_Once);
        ImGui::SetNextWindowSize(ImVec2(420.0f, 520.0f), ImGuiCond_Once);

        const ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoNav;

        ImGui::Begin("Kibako - Debug", nullptr, windowFlags);

        if (ImGui::BeginTabBar("KibakoDebugTabs")) {

            if (ImGui::BeginTabItem("Engine")) {
                const float fps = io.Framerate;
                const float frameMs = (fps > 0.0f) ? (1000.0f / fps) : 0.0f;
                const float deltaTime = io.DeltaTime;
                const ImVec2 display = io.DisplaySize;

                ImGui::Text("Frame: %.3f ms (%.1f FPS)", frameMs, fps);
                ImGui::Text("Delta time: %.5f s", deltaTime);

                ImGui::Separator();
                ImGui::Text("Renderer");
                ImGui::BulletText("Backbuffer: %.0f x %.0f", display.x, display.y);
                ImGui::BulletText("VSync: %s", g_vsyncEnabled ? "ON" : "OFF");
                ImGui::BulletText("Sprites: %u", g_renderStats.spritesSubmitted);
                ImGui::BulletText("Draw calls: %u", g_renderStats.drawCalls);

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
                ImGui::Text("Shortcuts");
                ImGui::BulletText("F2: toggle debug overlay");
                ImGui::BulletText("F1: toggle collision overlay (sandbox)");

                ImGui::EndTabItem();
            }

            if (ImGui::BeginTabItem("Performance")) {
                static float frameHistory[120] = {};
                static int   historyIndex = 0;
                const int    historySize = IM_ARRAYSIZE(frameHistory);

                const float frameMs = (io.Framerate > 0.0f) ? (1000.0f / io.Framerate) : 0.0f;
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
                    ImVec2(0.0f, 80.0f));
                ImGui::Text("Target: 16.6 ms (60 FPS)");

                ImGui::EndTabItem();
            }

            if (ImGui::BeginTabItem("Input")) {
                ImGui::Text("Mouse");
                ImGui::BulletText("Position: (%.0f, %.0f)", io.MousePos.x, io.MousePos.y);

                ImGui::Separator();
                ImGui::Text("Mouse buttons");
                ImGui::BulletText("Left:   %s", io.MouseDown[0] ? "Down" : "Up");
                ImGui::BulletText("Right:  %s", io.MouseDown[1] ? "Down" : "Up");
                ImGui::BulletText("Middle: %s", io.MouseDown[2] ? "Down" : "Up");

                ImGui::Separator();
                ImGui::TextDisabled("Gameplay input handled by KibakoEngine::Input");

                ImGui::EndTabItem();
            }

            if (ImGui::BeginTabItem("Scene")) {
                if (g_sceneInspectorCallback && g_sceneInspectorUserData) {
                    g_sceneInspectorCallback(g_sceneInspectorUserData);
                }
                else {
                    ImGui::TextDisabled("No Scene2D inspector registered.");
                }

                ImGui::EndTabItem();
            }

            ImGui::EndTabBar();
        }

        ImGui::End();

        ImGui::Render();
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
    }

    void SetEnabled(bool enabled)
    {
        g_enabled = enabled;
    }

    bool IsEnabled()
    {
        return g_enabled;
    }

    void ToggleEnabled()
    {
        g_enabled = !g_enabled;
    }

    void SetVSyncEnabled(bool enabled)
    {
        g_vsyncEnabled = enabled;
    }

    bool IsVSyncEnabled()
    {
        return g_vsyncEnabled;
    }

    void SetRenderStats(const RenderStats& stats)
    {
        g_renderStats = stats;
    }

    RenderStats GetRenderStats()
    {
        return g_renderStats;
    }

    void SetSceneInspector(void* userData, PanelCallback callback)
    {
        g_sceneInspectorUserData = userData;
        g_sceneInspectorCallback = callback;
    }

} // namespace KibakoEngine::DebugUI

#endif // KBK_DEBUG_BUILD
