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

    // --- Création des 3 entités ---

    // 1) Sprite gauche (bleu, pas de collider)
    {
        Entity2D& e = m_scene.CreateEntity();
        m_entityLeft = e.id;

        e.transform.position = { 80.0f, 140.0f };
        e.transform.rotation = 0.0f;
        e.transform.scale = { 1.0f, 1.0f };

        e.sprite.texture = m_starTexture;
        e.sprite.dst = RectF::FromXYWH(0.0f, 0.0f, width, height);
        e.sprite.src = RectF::FromXYWH(0.0f, 0.0f, 1.0f, 1.0f);
        e.sprite.color = Color4{ 0.25f, 0.8f, 1.0f, 1.0f };
        e.sprite.layer = -1;
    }

    // 2) Sprite centre (blanc) avec collider
    {
        Entity2D& e = m_scene.CreateEntity();
        m_entityCenter = e.id;

        e.transform.position = { 200.0f, 150.0f };
        e.transform.rotation = 0.0f;
        e.transform.scale = { 1.2f, 1.2f }; // un peu plus gros

        e.sprite.texture = m_starTexture;
        e.sprite.dst = RectF::FromXYWH(0.0f, 0.0f, width, height);
        e.sprite.src = RectF::FromXYWH(0.0f, 0.0f, 1.0f, 1.0f);
        e.sprite.color = Color4::White();
        e.sprite.layer = 0;

        // Collider circulaire centre
        m_centerCollider.radius = 0.5f * width * e.transform.scale.x;
        m_centerCollider.active = true;
        e.collision.circle = &m_centerCollider;
    }

    // 3) Sprite droite (orange) avec collider
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

        // Collider circulaire droite
        m_rightCollider.radius = 0.5f * width * e.transform.scale.x;
        m_rightCollider.active = true;
        e.collision.circle = &m_rightCollider;
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

    m_entityLeft = 0;
    m_entityCenter = 0;
    m_entityRight = 0;

    m_centerCollider = {};
    m_rightCollider = {};
}

void GameLayer::OnUpdate(float dt)
{
    KBK_PROFILE_SCOPE("GameLayerUpdate");

    m_time += dt;

    const float bobbing = std::sin(m_time * 2.0f) * 32.0f;
    const float sway = std::sin(m_time * 1.5f) * 200.0f;

    // Entité centre : mouvement + rotation
    Entity2D* center = m_scene.FindEntity(m_entityCenter);
    if (center) {
        center->transform.position.x = 200.0f + sway;
        center->transform.position.y = 150.0f + bobbing;
        center->transform.rotation = m_time * 0.8f;
    }

    // Entité droite : rotation inverse
    Entity2D* right = m_scene.FindEntity(m_entityRight);
    if (right) {
        right->transform.rotation = -m_time * 0.5f;
    }

    // Test de collision entre centre et droite
    bool hit = false;

    if (center && right &&
        center->collision.circle && right->collision.circle)
    {
        hit = Intersects(
            *center->collision.circle, center->transform,
            *right->collision.circle, right->transform
        );
    }

    // Feedback visuel simple : rouge si collision, sinon couleurs normales
    if (center) {
        if (hit)
            center->sprite.color = Color4{ 1.0f, 0.2f, 0.2f, 1.0f };
        else
            center->sprite.color = Color4::White();
    }

    if (right) {
        if (hit)
            right->sprite.color = Color4{ 1.0f, 0.4f, 0.2f, 1.0f };
        else
            right->sprite.color = Color4{ 1.0f, 0.55f, 0.35f, 1.0f };
    }

    if (hit) {
        KbkTrace(kLogChannel, "Center/Right COLLISION");
    }

    m_scene.Update(dt);
}

void GameLayer::OnRender(SpriteBatch2D& batch)
{
    KBK_PROFILE_SCOPE("GameLayerRender");

    if (!m_starTexture || !m_starTexture->IsValid())
        return;

    m_scene.Render(batch);
}