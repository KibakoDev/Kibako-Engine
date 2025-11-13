// Kibako2DSandbox/src/main.cpp

#ifndef SDL_MAIN_HANDLED
#define SDL_MAIN_HANDLED
#endif

#include <SDL2/SDL.h>

#include "KibakoEngine/Core/Application.h"
#include "KibakoEngine/Core/Log.h"
#include "KibakoEngine/Renderer/Texture2D.h"
#include "KibakoEngine/Renderer/SpriteTypes.h"

using namespace KibakoEngine;

struct StarInstance
{
    RectF  dst;
    Color4 tint;
    float  rotation;
    int    layer;
};

int main(int argc, char* argv[])
{
    (void)argc;
    (void)argv;

    Application app;
    if (!app.Init(960, 540, "KibakoEngine Sandbox")) {
        KbkLog("Sandbox", "Failed to initialize Application");
        return 1;
    }

    auto& renderer = app.Renderer();
    ID3D11Device* device = renderer.GetDevice();
    if (!device) {
        KbkLog("Sandbox", "Renderer device not ready after init");
        app.Shutdown();
        return 1;
    }

    Texture2D texStar;
    if (!texStar.LoadFromFile(device, "assets/star.png", true)) {
        KbkLog("Sandbox", "Failed to load texture: assets/star.png");
        app.Shutdown();
        return 1;
    }

    const RectF uvFull{ 0.0f, 0.0f, 1.0f, 1.0f };

    StarInstance stars[] = {
        {
            RectF{ 120.0f, 120.0f,
                   static_cast<float>(texStar.Width()),
                   static_cast<float>(texStar.Height()) },
            Color4::White(),
            0.0f,
            0
        },
        {
            RectF{ 320.0f, 160.0f,
                   static_cast<float>(texStar.Width()),
                   static_cast<float>(texStar.Height()) },
            Color4::White(),
            0.0f,
            1
        },
        {
            RectF{ 520.0f, 100.0f,
                   static_cast<float>(texStar.Width()),
                   static_cast<float>(texStar.Height()) },
            Color4::White(),
            0.0f,
            -1
        }
    };

    const float clearColor[4] = { 0.0f, 0.0f, 0.0f, 1.0f };

    while (app.PumpEvents())
    {
        app.BeginFrame(clearColor);

        auto& batch = renderer.Batch();
        batch.SetPixelSnap(true);
        batch.Begin(renderer.Camera().GetViewProjectionT());

        for (const auto& star : stars) {
            batch.Push(
                texStar,
                star.dst,
                uvFull,
                star.tint,
                star.rotation,
                star.layer
            );
        }

        batch.End();
        app.EndFrame();
    }

    app.Shutdown();
    return 0;
}