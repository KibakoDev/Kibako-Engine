#include "GameLayer.h"

#include "KibakoEngine/Core/Application.h"
#include "KibakoEngine/Core/Debug.h"
#include "KibakoEngine/Core/Log.h"
#include "KibakoEngine/Core/Profiler.h"
#include "KibakoEngine/Renderer/RendererD3D11.h"

#include <cmath>

using namespace KibakoEngine;

namespace
{
    constexpr const char* kLogChannel = "Sandbox";
}

GameLayer::GameLayer(Application& app)
    : Layer("Sandbox.GameLayer")
    , m_app(app)
{
    // Rien à faire ici pour l'instant, tout se passe dans OnAttach
}

void GameLayer::OnAttach()
{
    KBK_PROFILE_SCOPE("GameLayerAttach");

    auto& assets = m_app.Assets();

    m_starTexture = assets.LoadTexture("star", "assets/star.png", true);
    if (!m_starTexture || !m_starTexture->IsValid()) {
        KbkError(kLogChannel, "Failed to load texture: assets/star.png");
        return;
    }

    const float width = static_cast<float>(m_starTexture->Width());
    const float height = static_cast<float>(m_starTexture->Height());

    // --- Création des 3 entités équivalentes à tes anciens SampleSprite ---

    // 1) Sprite à gauche (bleu)
    {
        Entity2D& e = m_scene.CreateEntity();
        m_entityCenter = e.id;

        e.transform.position = { 80.0f, 140.0f };
        e.transform.rotation = 0.0f;
        e.transform.scale = { 1.0f, 1.0f };

        e.sprite.texture = m_starTexture;
        e.sprite.dst = RectF::FromXYWH(0.0f, 0.0f, width, height);
        e.sprite.src = RectF::FromXYWH(0.0f, 0.0f, 1.0f, 1.0f);
        e.sprite.color = Color4{ 0.25f, 0.8f, 1.0f, 1.0f };
        e.sprite.layer = -1;
    }

    // 2) Sprite au centre (blanc) qui va bouger
    {
        Entity2D& e = m_scene.CreateEntity();
        m_entityBobbing = e.id;

        e.transform.position = { 200.0f, 150.0f };
        e.transform.rotation = 0.0f;
        e.transform.scale = { 1.0f, 1.0f };

        e.sprite.texture = m_starTexture;
        e.sprite.dst = RectF::FromXYWH(0.0f, 0.0f, width, height);
        e.sprite.src = RectF::FromXYWH(0.0f, 0.0f, 1.0f, 1.0f);
        e.sprite.color = Color4::White();
        e.sprite.layer = 0;
    }

    // 3) Sprite à droite (orange), rotation inverse
    {
        Entity2D& e = m_scene.CreateEntity();
        m_entityRight = e.id;

        e.transform.position = { 340.0f, 160.0f };
        e.transform.rotation = 0.0f;
        e.transform.scale = { 1.0f, 1.0f };

        e.sprite.texture = m_starTexture;
        e.sprite.dst = RectF::FromXYWH(0.0f, 0.0f, width, height);
        e.sprite.src = RectF::FromXYWH(0.0f, 0.0f, 1.0f, 1.0f);
        e.sprite.color = Color4{ 1.0f, 0.55f, 0.35f, 1.0f };
        e.sprite.layer = 1;
    }

    KbkLog(kLogChannel,
        "GameLayer attached (%d x %d texture, %zu entities)",
        m_starTexture->Width(),
        m_starTexture->Height(),
        m_scene.Entities().size());
}

void GameLayer::OnDetach()
{
    KBK_PROFILE_SCOPE("GameLayerDetach");

    m_starTexture = nullptr;
    m_scene.Clear();
    m_entityCenter = 0;
    m_entityBobbing = 0;
    m_entityRight = 0;
}

void GameLayer::OnUpdate(float dt)
{
    KBK_PROFILE_SCOPE("GameLayerUpdate");

    m_time += dt;

    // Mouvement et rotation sur certaines entités pour tester Scene2D

    const float bobbing = std::sin(m_time * 2.0f) * 32.0f;
    const float sway = std::sin(m_time * 1.5f) * 200.0f;

    // Entité "bobbing" (celle du centre)
    if (auto* e = m_scene.FindEntity(m_entityBobbing)) {
        e->transform.position.x = 200.0f + sway;
        e->transform.position.y = 150.0f + bobbing;
        e->transform.rotation = m_time * 0.8f;
    }

    // Entité de droite : légère rotation inverse
    if (auto* e = m_scene.FindEntity(m_entityRight)) {
        e->transform.rotation = -m_time * 0.5f;
    }

    // La scène en elle-même n'a pas de logique interne pour l'instant
    m_scene.Update(dt);
}

void GameLayer::OnRender(SpriteBatch2D& batch)
{
    KBK_PROFILE_SCOPE("GameLayerRender");

    if (!m_starTexture || !m_starTexture->IsValid())
        return;

    m_scene.Render(batch);
}