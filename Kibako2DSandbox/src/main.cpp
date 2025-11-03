// Kibako2DSandbox/src/main.cpp
#define SDL_MAIN_HANDLED
#include "KibakoEngine/Core/Application.h"
#include "KibakoEngine/Renderer/Texture2D.h"
#include "KibakoEngine/Renderer/SpriteTypes.h"

using namespace KibakoEngine;

int main()
{
    Application app;
    if (!app.Init(1280, 720, "Kibako Sandbox")) {
        return 1;
    }

    // Example game-side resources
    Texture2D ship;
    ship.LoadFromFile(app.Renderer().GetDevice(), "assets/star.png", true);

    // Simple camera setup
    auto& cam = app.Renderer().Camera();
    cam.SetPosition(0.0f, 0.0f);

    // Main loop driven by Sandbox
    while (app.PumpEvents())
    {
        // Input-driven camera (optional: keep or remove)
        const float dt = (float)app.TimeSys().DeltaSeconds();
        const float move = 600.0f * dt;
        if (app.InputSys().KeyDown(SDL_SCANCODE_W)) cam.Move(0.0f, -move);
        if (app.InputSys().KeyDown(SDL_SCANCODE_S)) cam.Move(0.0f, move);
        if (app.InputSys().KeyDown(SDL_SCANCODE_A)) cam.Move(-move, 0.0f);
        if (app.InputSys().KeyDown(SDL_SCANCODE_D)) cam.Move(move, 0.0f);
        if (app.InputSys().KeyDown(SDL_SCANCODE_Q)) cam.AddRotation(-1.5f * dt);
        if (app.InputSys().KeyDown(SDL_SCANCODE_E)) cam.AddRotation(1.5f * dt);
        if (app.InputSys().KeyDown(SDL_SCANCODE_R)) cam.Reset();

        // Render
        app.BeginFrame();

        auto& sprites = app.Renderer().Sprites();
        sprites.Begin(app.Renderer().Camera().GetViewProjT());

        if (ship.GetSRV()) {
            RectF  dst{ 200.0f, 150.0f, (float)ship.Width(), (float)ship.Height() };
            RectF  src{ 0.0f, 0.0f, 1.0f, 1.0f };
            Color4 tint = Color4::White();
            sprites.SetMonochrome(0.0f);
            sprites.DrawSprite(ship, dst, src, tint, 0.0f);
        }

        sprites.End();
        app.EndFrame();
    }

    app.Shutdown();
    return 0;
}