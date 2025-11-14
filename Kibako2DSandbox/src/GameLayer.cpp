#include "GameLayer.h"

#include "KibakoEngine/Core/Application.h"
#include "KibakoEngine/Core/Debug.h"
#include "KibakoEngine/Core/Log.h"
#include "KibakoEngine/Core/Profiler.h"
#include "KibakoEngine/Core/DebugUI.h"
#include "KibakoEngine/Core/GameServices.h"
#include "KibakoEngine/Renderer/DebugDraw2D.h"

#include <DirectXMath.h>
#include <SDL2/SDL_scancode.h>
#include <cstddef>
#include <cmath>
#include <cstdio>

#include "imgui.h"

namespace
{
    constexpr const char* kLogChannel = "Sandbox";
    constexpr int   kDebugDrawLayer = 1000;
    constexpr float kColliderThickness = 2.0f;

    // ==========================
    // Scene / Entity Inspector panel
    // ==========================
    void SceneInspectorPanel(void* userData)
    {
        auto* scene = static_cast<KibakoEngine::Scene2D*>(userData);
        if (!scene)
            return;

        auto& entities = scene->Entities();

        static int selectedIndex = -1;
        if (selectedIndex >= static_cast<int>(entities.size()))
            selectedIndex = -1;

        ImGui::Begin("Kibako - Scene2D");

        ImGui::Text("Entities: %d", static_cast<int>(entities.size()));
        ImGui::Separator();

        if (ImGui::BeginListBox("Entities")) {
            for (int i = 0; i < static_cast<int>(entities.size()); ++i) {
                const KibakoEngine::Entity2D& e = entities[static_cast<std::size_t>(i)];

                char label[64];
                std::snprintf(label, sizeof(label), "ID %u%s",
                    e.id,
                    e.active ? "" : " (disabled)");

                bool isSelected = (selectedIndex == i);
                if (ImGui::Selectable(label, isSelected)) {
                    selectedIndex = i;
                }
                if (isSelected)
                    ImGui::SetItemDefaultFocus();
            }
            ImGui::EndListBox();
        }

        ImGui::Separator();

        if (selectedIndex >= 0 && selectedIndex < static_cast<int>(entities.size())) {
            KibakoEngine::Entity2D& e = entities[static_cast<std::size_t>(selectedIndex)];

            ImGui::Text("Selected ID: %u", e.id);

            ImGui::Checkbox("Active", &e.active);

            KibakoEngine::Transform2D& t = e.transform;

            ImGui::Text("Transform2D");
            ImGui::DragFloat2("Position", &t.position.x, 1.0f);
            ImGui::DragFloat("Rotation (rad)", &t.rotation, 0.01f);
            ImGui::DragFloat2("Scale", &t.scale.x, 0.01f, 0.01f, 10.0f);
        }
        else {
            ImGui::TextDisabled("No entity selected.");
        }

        ImGui::End();
    }

} // anonymous namespace

GameLayer::GameLayer(KibakoEngine::Application& app)
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
    const KibakoEngine::RectF spriteRect =
        KibakoEngine::RectF::FromXYWH(0.0f, 0.0f, width, height);
    const KibakoEngine::RectF uvRect =
        KibakoEngine::RectF::FromXYWH(0.0f, 0.0f, 1.0f, 1.0f);

    auto configureSprite = [&](KibakoEngine::Entity2D& entity,
        const DirectX::XMFLOAT2& position,
        const DirectX::XMFLOAT2& scale,
        const KibakoEngine::Color4& color,
        int layer)
        {
            entity.transform.position = position;
            entity.transform.rotation = 0.0f;
            entity.transform.scale = scale;

            entity.sprite.texture = m_starTexture;
            entity.sprite.dst = spriteRect;
            entity.sprite.src = uvRect;
            entity.sprite.color = color;
            entity.sprite.layer = layer;
        };

    // Left sprite (blue, no collider)
    {
        KibakoEngine::Entity2D& entity = m_scene.CreateEntity();
        configureSprite(entity,
            { 80.0f, 140.0f },
            { 1.0f, 1.0f },
            KibakoEngine::Color4{ 0.25f, 0.8f, 1.0f, 1.0f },
            -1);
    }

    // Center sprite (white) with collider
    {
        KibakoEngine::Entity2D& entity = m_scene.CreateEntity();
        m_entityCenter = entity.id;
        configureSprite(entity,
            { 200.0f, 150.0f },
            { 1.2f, 1.2f },
            KibakoEngine::Color4::White(),
            0);

        m_centerCollider.radius = 0.5f * width * entity.transform.scale.x;
        m_centerCollider.active = true;
        entity.collision.circle = &m_centerCollider;
    }

    // Right sprite (orange) with collider
    {
        KibakoEngine::Entity2D& entity = m_scene.CreateEntity();
        m_entityRight = entity.id;
        configureSprite(entity,
            { 340.0f, 160.0f },
            { 1.0f, 1.0f },
            KibakoEngine::Color4{ 1.0f, 0.55f, 0.35f, 1.0f },
            1);

        m_rightCollider.radius = 0.5f * width * entity.transform.scale.x;
        m_rightCollider.active = true;
        entity.collision.circle = &m_rightCollider;
    }

    KbkLog(kLogChannel,
        "GameLayer attached (%d x %d texture, %zu entities)",
        m_starTexture->Width(),
        m_starTexture->Height(),
        m_scene.Entities().size());

    // Register the Scene Inspector panel with the debug UI.
    KibakoEngine::DebugUI::SetSceneInspector(&m_scene, &SceneInspectorPanel);
}

void GameLayer::OnDetach()
{
    KBK_PROFILE_SCOPE("GameLayerDetach");

    m_starTexture = nullptr;
    m_scene.Clear();

    m_entityCenter = 0;
    m_entityRight = 0;

    m_centerCollider = {};
    m_rightCollider = {};

    m_showCollisionDebug = false;
    m_lastCollision = false;
    m_time = 0.0f;

    KibakoEngine::DebugUI::SetSceneInspector(nullptr, nullptr);
}

void GameLayer::OnUpdate(float dt)
{
    KBK_PROFILE_SCOPE("GameLayerUpdate");

    // Toggle debug collisions
    auto& input = m_app.InputSys();
    if (input.KeyPressed(SDL_SCANCODE_F1)) {
        m_showCollisionDebug = !m_showCollisionDebug;
        KbkTrace(kLogChannel, "Collision debug %s", m_showCollisionDebug ? "ON" : "OFF");
    }

    // === GameServices: temps global (timeScale / pause) ===
    KibakoEngine::GameServices::Update(static_cast<double>(dt));
    const double scaledDt = KibakoEngine::GameServices::GetScaledDeltaTime();
    const float  fdt = static_cast<float>(scaledDt);

    // On avance notre "time" sandbox avec le temps SCALÉ
    m_time += fdt;

    const float bobbing = std::sin(m_time * 2.0f) * 32.0f;
    const float sway = std::sin(m_time * 0.2f) * 300.0f;

    auto* centerEntity = m_scene.FindEntity(m_entityCenter);
    auto* rightEntity = m_scene.FindEntity(m_entityRight);

    // Center entity: movement + rotation
    if (centerEntity) {
        centerEntity->transform.position.x = 200.0f + sway;
        centerEntity->transform.position.y = 150.0f + bobbing;
        centerEntity->transform.rotation = m_time * 0.8f;
    }

    // Right entity: counter rotation
    if (rightEntity) {
        rightEntity->transform.rotation = -m_time * 0.5f;
    }

    bool hit = false;
    if (centerEntity && rightEntity &&
        centerEntity->collision.circle && rightEntity->collision.circle) {

        hit = KibakoEngine::Intersects(*centerEntity->collision.circle, centerEntity->transform,
            *rightEntity->collision.circle, rightEntity->transform);
    }

    // Visual feedback: switch colours when colliding
    if (centerEntity) {
        centerEntity->sprite.color = hit
            ? KibakoEngine::Color4{ 1.0f, 0.2f, 0.2f, 1.0f }
        : KibakoEngine::Color4::White();
    }

    if (rightEntity) {
        rightEntity->sprite.color = hit
            ? KibakoEngine::Color4{ 1.0f, 0.4f, 0.2f, 1.0f }
        : KibakoEngine::Color4{ 1.0f, 0.55f, 0.35f, 1.0f };
    }

    if (hit) {
        KbkTrace(kLogChannel, "Center/Right COLLISION");
    }

    m_lastCollision = hit;

    // La scène aussi est mise à jour avec le temps SCALÉ
    m_scene.Update(fdt);
}

void GameLayer::OnRender(KibakoEngine::SpriteBatch2D& batch)
{
    KBK_PROFILE_SCOPE("GameLayerRender");

    if (!m_starTexture || !m_starTexture->IsValid())
        return;

    m_scene.Render(batch);

    if (m_showCollisionDebug) {
        const KibakoEngine::Color4 circleColor = m_lastCollision
            ? KibakoEngine::Color4{ 1.0f, 0.3f, 0.3f, 1.0f }
        : KibakoEngine::Color4{ 0.2f, 0.9f, 0.9f, 1.0f };

        const KibakoEngine::Color4 crossColor =
            KibakoEngine::Color4{ 1.0f, 1.0f, 0.4f, 1.0f };
        const KibakoEngine::Color4 aabbColor =
            KibakoEngine::Color4{ 0.9f, 0.9f, 0.2f, 1.0f };

        for (const KibakoEngine::Entity2D& entity : m_scene.Entities()) {
            if (!entity.active)
                continue;

            const KibakoEngine::Transform2D& transform = entity.transform;

            const bool drewCollider =
                KibakoEngine::DebugDraw2D::DrawCollisionComponent(batch,
                    transform,
                    entity.collision,
                    circleColor,
                    aabbColor,
                    kColliderThickness,
                    kDebugDrawLayer,
                    48);

            if (drewCollider) {
                KibakoEngine::DebugDraw2D::DrawCross(batch,
                    transform.position,
                    10.0f,
                    crossColor,
                    kColliderThickness,
                    kDebugDrawLayer);
            }
        }
    }
}