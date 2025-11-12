#define SDL_MAIN_HANDLED
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN

#include <SDL2/SDL.h>

#include "KibakoEngine/Core/Application.h"
#include "KibakoEngine/Core/Log.h"
#include "KibakoEngine/Renderer/SpriteTypes.h"
#include "KibakoEngine/Renderer/Texture2D.h"

using namespace KibakoEngine;

int main()
{
    Application app;
    if (!app.Init(960, 540, "KibakoEngine Sandbox")) {
        return 1;
    }

    auto& renderer = app.Renderer();
    auto* device = renderer.GetDevice();
    if (!device) {
        KbkLog("Sandbox", "Renderer device not ready");
        app.Shutdown();
        return 1;
    }

    Texture2D texStar;
    if (!texStar.LoadFromFile(device, "assets/star.png", true)) {
        KbkLog("Sandbox", "Failed to load assets/star.png");
        app.Shutdown();
        return 1;
    }

    RectF dstStar1{ 120.0f, 120.0f, static_cast<float>(texStar.Width()), static_cast<float>(texStar.Height()) };
    RectF dstStar2{ 320.0f, 160.0f, static_cast<float>(texStar.Width()), static_cast<float>(texStar.Height()) };
    RectF dstStar3{ 520.0f, 100.0f, static_cast<float>(texStar.Width()), static_cast<float>(texStar.Height()) };
    const RectF uvFull{ 0.0f, 0.0f, 1.0f, 1.0f };

    bool monochrome = false;
    bool pointSampling = true;

    const float clearColor[4] = { 0.05f, 0.06f, 0.08f, 1.0f };

    while (app.PumpEvents()) {
        if (app.InputSys().KeyPressed(SDL_SCANCODE_M)) {
            monochrome = !monochrome;
            KbkLog("Sandbox", "Monochrome %s", monochrome ? "ON" : "OFF");
        }
        if (app.InputSys().KeyPressed(SDL_SCANCODE_P)) {
            pointSampling = !pointSampling;
            KbkLog("Sandbox", "Point sampling %s", pointSampling ? "ON" : "OFF");
        }

        app.BeginFrame(clearColor);

        auto& batch = renderer.Batch();
        batch.SetMonochrome(monochrome ? 1.0f : 0.0f);
        batch.SetPointSampling(pointSampling);
        batch.SetPixelSnap(true);
        batch.Begin(renderer.Camera().GetViewProjectionT());

        batch.Push(texStar, dstStar3, uvFull, Color4{ 0.2f, 0.8f, 1.0f, 1.0f }, 0.0f, -1);
        batch.Push(texStar, dstStar1, uvFull, Color4::White(), 0.0f, 0);
        batch.Push(texStar, dstStar2, uvFull, Color4{ 1.0f, 0.5f, 0.3f, 1.0f }, 0.35f, 1);

        batch.End();
        app.EndFrame();
    }

    app.Shutdown();
    return 0;
}

