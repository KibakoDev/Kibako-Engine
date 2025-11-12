// Kibako2DSandbox/src/main.cpp
#define SDL_MAIN_HANDLED
#define NOMINMAX
#include <SDL2/SDL.h>
#include <iostream>
#include <string>

#include "KibakoEngine/Core/Application.h"
#include "KibakoEngine/Renderer/Texture2D.h"
#include "KibakoEngine/Renderer/SpriteTypes.h"

#include <vector>
#include <algorithm>
#include <cstdlib>
#include <ctime>

// Petit struct jeu: vaisseau + obstacles
struct Asteroid {
    KibakoEngine::RectF rect;
    float speed;
    float rotation;
};

using namespace KibakoEngine;

int main() {
    std::srand(static_cast<unsigned int>(std::time(nullptr)));

    Application app;
    if (!app.Init(1280, 720, "KibakoEngine - Sandbox")) {
        return 1;
    }

    // Textures (rutilise star.png si tu nas pas dasteroid.png)
    Texture2D texShip;
    Texture2D texAst;
    auto* device = app.Renderer().GetDevice();
    if (!device) {
        std::cerr << "Renderer device is null, aborting sandbox" << std::endl;
        app.Shutdown();
        return 1;
    }

    auto loadTexture = [&](Texture2D& tex, const char* path, const char* label, bool critical) {
        if (tex.LoadFromFile(device, path, true)) {
            return true;
        }
        std::string msg = std::string("Echec du chargement de ") + label + " (" + path + ")";
        std::cerr << msg << std::endl;
        if (critical) {
            SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Erreur de texture", msg.c_str(), nullptr);
        }
        return false;
    };

    if (!loadTexture(texShip, "assets/star.png", "la texture du vaisseau", true)) {
        app.Shutdown();
        return 1;
    }

    if (!loadTexture(texAst, "assets/asteroid.png", "les asteroides", false)) {
        texAst = texShip;
    }

    // Monde / caméra: fixe
    const int winW = 1280;
    const int winH = 720;
    auto& cam = app.Renderer().Camera();
    cam.SetPosition(0.0f, 0.0f);     // origine haut-gauche (selon ton Camera2D)
    cam.SetRotation(0.0f);           // pas de rotation

    // Vaisseau: centré verticalement, 10% du bord gauche
    RectF ship{
        winW * 0.10f,
        winH * 0.5f - texShip.Height() * 0.5f,
        (float)texShip.Width(),
        (float)texShip.Height()
    };
    const float shipSpeed = 450.0f; // px/s

    // Astéroïdes
    std::vector<Asteroid> asts;
    float spawnTimer = 0.0f;
    const float autoSpawnEvery = 0.6f; // s

    // Rendu batch: pixels nets
    auto& batch = app.Renderer().Batch();
    batch.SetPointSampling(true);
    batch.SetPixelSnap(true);
    batch.SetMonochrome(0.0f);

    while (app.PumpEvents()) {
        const float dt = (float)app.TimeSys().DeltaSeconds();

        // ---- Input vaisseau: W/S ou flèches haut/bas ----
        float dy = 0.0f;
        if (app.InputSys().KeyDown(SDL_SCANCODE_W) || app.InputSys().KeyDown(SDL_SCANCODE_UP))   dy -= shipSpeed * dt;
        if (app.InputSys().KeyDown(SDL_SCANCODE_S) || app.InputSys().KeyDown(SDL_SCANCODE_DOWN)) dy += shipSpeed * dt;

        ship.y = std::clamp(ship.y + dy, 0.0f, (float)winH - ship.h);

        // ---- Spawn astéroïdes ----
        spawnTimer += dt;
        auto spawnOne = [&]() {
            const float h = (float)texAst.Height();
            const float w = (float)texAst.Width();
            // position X au bord droit, Y random (marge pour éviter bord)
            float y = (float)(20 + std::rand() % (winH - 40));
            float speed = 220.0f + (std::rand() % 180); // 220..400 px/s
            float rot = ((std::rand() % 100) / 100.0f) * 0.4f - 0.2f; // -0.2..0.2 rad/s
            asts.push_back({ RectF{ (float)winW + w, y - h * 0.5f, w, h }, speed, rot });
            };

        if (spawnTimer >= autoSpawnEvery) {
            spawnTimer = 0.0f;
            spawnOne();
        }
        if (app.InputSys().KeyPressed(SDL_SCANCODE_SPACE)) {
            // spawn manuel: petit burst
            for (int i = 0; i < 3; ++i) spawnOne();
        }

        // ---- Update astéroïdes ----
        for (auto& a : asts) {
            a.rect.x -= a.speed * dt;
            // Option: légère rotation visuelle (si tu veux)
            a.rotation += 0.6f * dt;
        }
        // purge hors-écran
        asts.erase(
            std::remove_if(asts.begin(), asts.end(),
                [&](const Asteroid& a) { return a.rect.x + a.rect.w < -100.0f; }),
            asts.end()
        );

        // ---- (Option) Collision AABB simple pour test ----
        // juste un print console si contact
        for (auto& a : asts) {
            bool overlap =
                ship.x < a.rect.x + a.rect.w && ship.x + ship.w > a.rect.x &&
                ship.y < a.rect.y + a.rect.h && ship.y + ship.h > a.rect.y;
            if (overlap) {
                // pas de game-over ici, juste un ping (évite flood en vrai)
                // printf("HIT!\n");
                break;
            }
        }

        // ---- Render ----
        app.BeginFrame();

        batch.Begin(app.Renderer().Camera().GetViewProjT());

}
        if (texShip.GetSRV()) {
            RectF src{ 0,0,1,1 };
            batch.Push(texShip, ship, src, Color4::White(), 0.0f, 10);
        }

        // draw asteroids (layer 0)
        if (texAst.GetSRV()) {
            RectF src{ 0,0,1,1 };
            for (auto& a : asts) {
                batch.Push(texAst, a.rect, src, Color4::White(), a.rotation, 0);
            }
        }

        batch.End();
        app.EndFrame();
    }

    app.Shutdown();
    return 0;
}