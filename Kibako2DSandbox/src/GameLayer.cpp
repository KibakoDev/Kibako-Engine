#include "GameLayer.h"

#include "KibakoEngine/Core/Application.h"
#include "KibakoEngine/Core/Debug.h"
#include "KibakoEngine/Core/Log.h"
#include "KibakoEngine/Core/Profiler.h"
#include "KibakoEngine/Renderer/RendererD3D11.h"

#include <DirectXMath.h>
#include <SDL2/SDL_scancode.h>
#include <cmath>

using namespace KibakoEngine;

namespace
{
    constexpr const char* kLogChannel = "Sandbox";
    constexpr int kDebugDrawLayer = 1000;
    constexpr float kColliderThickness = 2.0f;

    const RectF kUnitRect = RectF::FromXYWH(0.0f, 0.0f, 1.0f, 1.0f);

    void DrawLine(SpriteBatch2D& batch,
        Texture2D& pixel,
        const DirectX::XMFLOAT2& a,
        const DirectX::XMFLOAT2& b,
        const Color4& color,
        float thickness)
    {
        const float dx = b.x - a.x;
        const float dy = b.y - a.y;
        const float length = std::sqrt(dx * dx + dy * dy);
        if (length <= 0.0001f)
            return;

        RectF dst{};
        dst.w = length;
        dst.h = thickness;

        const float midX = (a.x + b.x) * 0.5f;
        const float midY = (a.y + b.y) * 0.5f;
        dst.x = midX - dst.w * 0.5f;
        dst.y = midY - dst.h * 0.5f;

        const float angle = std::atan2(dy, dx);

        batch.Push(pixel, dst, kUnitRect, color, angle, kDebugDrawLayer);
    }

    void DrawCross(SpriteBatch2D& batch,
        Texture2D& pixel,
        const DirectX::XMFLOAT2& center,
        float size,
        const Color4& color,
        float thickness)
    {
        const float half = size * 0.5f;
        const DirectX::XMFLOAT2 left{ center.x - half, center.y };
        const DirectX::XMFLOAT2 right{ center.x + half, center.y };
        const DirectX::XMFLOAT2 top{ center.x, center.y - half };
        const DirectX::XMFLOAT2 bottom{ center.x, center.y + half };

        DrawLine(batch, pixel, left, right, color, thickness);
        DrawLine(batch, pixel, top, bottom, color, thickness);
    }

    void DrawCircleOutline(SpriteBatch2D& batch,
        Texture2D& pixel,
        const DirectX::XMFLOAT2& center,
        float radius,
        const Color4& color,
        float thickness)
    {
        if (radius <= 0.0f)
            return;

        constexpr int segments = 32;
        DirectX::XMFLOAT2 prev{ center.x + radius, center.y };

        for (int i = 1; i <= segments; ++i) {
            const float angle = (static_cast<float>(i) / static_cast<float>(segments)) * 6.28318530717958647692f;
            DirectX::XMFLOAT2 next{
                center.x + std::cos(angle) * radius,
                center.y + std::sin(angle) * radius
            };
            DrawLine(batch, pixel, prev, next, color, thickness);
            prev = next;
        }
    }

    void DrawAABBOutline(SpriteBatch2D& batch,
        Texture2D& pixel,
        const DirectX::XMFLOAT2& center,
        float halfW,
        float halfH,
        const Color4& color,
        float thickness)
    {
        const DirectX::XMFLOAT2 tl{ center.x - halfW, center.y - halfH };
        const DirectX::XMFLOAT2 tr{ center.x + halfW, center.y - halfH };
        const DirectX::XMFLOAT2 br{ center.x + halfW, center.y + halfH };
        const DirectX::XMFLOAT2 bl{ center.x - halfW, center.y + halfH };

        DrawLine(batch, pixel, tl, tr, color, thickness);
        DrawLine(batch, pixel, tr, br, color, thickness);
        DrawLine(batch, pixel, br, bl, color, thickness);
        DrawLine(batch, pixel, bl, tl, color, thickness);
    }
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

    if (ID3D11Device* device = m_app.Renderer().GetDevice()) {
        if (!m_debugPixel.CreateSolidColor(device, 255, 255, 255, 255)) {
            KbkError(kLogChannel, "Failed to create debug pixel texture");
        }
    } else {
        KbkError(kLogChannel, "Renderer device unavailable for debug pixel creation");
    }

    const float width = static_cast<float>(m_starTexture->Width());
    const float height = static_cast<float>(m_starTexture->Height());

    // --- Cration des 3 entits ---

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
    m_debugPixel.Reset();
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

    auto& input = m_app.InputSys();
    if (input.KeyPressed(SDL_SCANCODE_F1)) {
        m_showCollisionDebug = !m_showCollisionDebug;
        KbkTrace(kLogChannel, "Collision debug %s", m_showCollisionDebug ? "ON" : "OFF");
    }

    m_time += dt;

    const float bobbing = std::sin(m_time * 2.0f) * 32.0f;
    const float sway = std::sin(m_time * 0.2f) * 300.0f;

    // Entit centre : mouvement + rotation
    Entity2D* center = m_scene.FindEntity(m_entityCenter);
    if (center) {
        center->transform.position.x = 200.0f + sway;
        center->transform.position.y = 150.0f + bobbing;
        center->transform.rotation = m_time * 0.8f;
    }

    // Entit droite : rotation inverse
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

    m_lastCollision = hit;

    m_scene.Update(dt);
}

void GameLayer::OnRender(SpriteBatch2D& batch)
{
    KBK_PROFILE_SCOPE("GameLayerRender");

    if (!m_starTexture || !m_starTexture->IsValid())
        return;

    m_scene.Render(batch);

    if (m_showCollisionDebug && m_debugPixel.IsValid()) {
        const Color4 circleColor = m_lastCollision
            ? Color4{ 1.0f, 0.3f, 0.3f, 1.0f }
            : Color4{ 0.2f, 0.9f, 0.9f, 1.0f };
        const Color4 crossColor = Color4{ 1.0f, 1.0f, 0.4f, 1.0f };
        const Color4 aabbColor = Color4{ 0.9f, 0.9f, 0.2f, 1.0f };

        for (const Entity2D& entity : m_scene.Entities()) {
            if (!entity.active)
                continue;

            const Transform2D& transform = entity.transform;

            if (entity.collision.circle && entity.collision.circle->active) {
                DrawCircleOutline(batch, m_debugPixel, transform.position, entity.collision.circle->radius, circleColor, kColliderThickness);
                DrawCross(batch, m_debugPixel, transform.position, 10.0f, crossColor, kColliderThickness);
            }

            if (entity.collision.aabb && entity.collision.aabb->active) {
                DrawAABBOutline(batch, m_debugPixel, transform.position, entity.collision.aabb->halfW, entity.collision.aabb->halfH, aabbColor, kColliderThickness);
                DrawCross(batch, m_debugPixel, transform.position, 10.0f, crossColor, kColliderThickness);
            }
        }
    }
}
