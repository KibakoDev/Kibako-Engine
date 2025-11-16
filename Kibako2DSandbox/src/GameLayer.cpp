// GameLayer.cpp - Implements the sandbox gameplay layer and scene inspector UI.
#include "GameLayer.h"

#include "KibakoEngine/Core/Application.h"
#include "KibakoEngine/Core/Debug.h"
#include "KibakoEngine/Core/DebugUI.h"
#include "KibakoEngine/Core/Log.h"
#include "KibakoEngine/Core/Profiler.h"
#include "KibakoEngine/Renderer/DebugDraw2D.h"

#include <DirectXMath.h>
#include <SDL2/SDL_scancode.h>

#include <cmath>
#include <cstdio>
#include <memory>

#if KBK_DEBUG_BUILD
#    include "imgui.h"
#endif

namespace
{
    constexpr const char* kLogChannel = "Sandbox";
    constexpr int   kDebugDrawLayer = 1000;
    constexpr float kColliderThickness = 2.0f;

#if KBK_DEBUG_BUILD
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
                const KibakoEngine::Entity2D& entity = entities[static_cast<std::size_t>(i)];

                char label[64];
                std::snprintf(label, sizeof(label), "ID %u%s",
                    entity.id,
                    entity.active ? "" : " (disabled)");

                const bool isSelected = (selectedIndex == i);
                if (ImGui::Selectable(label, isSelected))
                    selectedIndex = i;
                if (isSelected)
                    ImGui::SetItemDefaultFocus();
            }
            ImGui::EndListBox();
        }

        ImGui::Separator();

        if (selectedIndex >= 0 && selectedIndex < static_cast<int>(entities.size())) {
            KibakoEngine::Entity2D& entity = entities[static_cast<std::size_t>(selectedIndex)];

            ImGui::Text("Selected ID: %u", entity.id);
            ImGui::Checkbox("Active", &entity.active);

            KibakoEngine::Transform2D& transform = entity.transform;
            ImGui::Text("Transform2D");
            ImGui::DragFloat2("Position", &transform.position.x, 1.0f);
            ImGui::DragFloat("Rotation (rad)", &transform.rotation, 0.01f);
            ImGui::DragFloat2("Scale", &transform.scale.x, 0.01f, 0.01f, 10.0f);
        }
        else {
            ImGui::TextDisabled("No entity selected.");
        }

        ImGui::End();
    }
#endif

} // namespace

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

    m_uiFont = assets.LoadFontTTF("ui.default", "assets/fonts/RobotoMono-Regular.ttf", 32);
    if (!m_uiFont) {
        KbkWarn(kLogChannel, "Failed to load font: assets/fonts/RobotoMono-Regular.ttf");
    }

    m_uiSystem.SetInput(&m_app.InputSys());
    m_uiSystem.SetScreenSize(static_cast<float>(m_app.Width()), static_cast<float>(m_app.Height()));
    BuildUI();

    const float width = static_cast<float>(m_starTexture->Width());
    const float height = static_cast<float>(m_starTexture->Height());
    const KibakoEngine::RectF spriteRect = KibakoEngine::RectF::FromXYWH(0.0f, 0.0f, width, height);
    const KibakoEngine::RectF uvRect = KibakoEngine::RectF::FromXYWH(0.0f, 0.0f, 1.0f, 1.0f);

    auto configureSprite = [&](KibakoEngine::Entity2D& entity,
        const DirectX::XMFLOAT2& position,
        const DirectX::XMFLOAT2& scale,
        const KibakoEngine::Color4& color,
        int layer) {
            entity.transform.position = position;
            entity.transform.rotation = 0.0f;
            entity.transform.scale = scale;

            entity.sprite.texture = m_starTexture;
            entity.sprite.dst = spriteRect;
            entity.sprite.src = uvRect;
            entity.sprite.color = color;
            entity.sprite.layer = layer;
        };

    {
        KibakoEngine::Entity2D& entity = m_scene.CreateEntity();
        configureSprite(entity,
            { 80.0f, 140.0f },
            { 1.0f, 1.0f },
            KibakoEngine::Color4{ 0.25f, 0.8f, 1.0f, 1.0f },
            -1);
    }

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

#if KBK_DEBUG_BUILD
    KibakoEngine::DebugUI::SetSceneInspector(&m_scene, &SceneInspectorPanel);
#endif
}

void GameLayer::OnDetach()
{
    KBK_PROFILE_SCOPE("GameLayerDetach");

    m_starTexture = nullptr;
    m_uiFont = nullptr;
    m_scoreLabel = nullptr;
    m_hintLabel = nullptr;
    m_hudScreen = nullptr;
    m_menuScreen = nullptr;
    m_uiSystem.Clear();
    m_menuVisible = true;
    m_scene.Clear();

    m_entityCenter = 0;
    m_entityRight = 0;

    m_centerCollider = {};
    m_rightCollider = {};

    m_showCollisionDebug = false;
    m_lastCollision = false;
    m_time = 0.0f;

#if KBK_DEBUG_BUILD
    KibakoEngine::DebugUI::SetSceneInspector(nullptr, nullptr);
#endif
}

void GameLayer::OnUpdate(float dt)
{
    KBK_PROFILE_SCOPE("GameLayerUpdate");

    auto& input = m_app.InputSys();
    if (input.KeyPressed(SDL_SCANCODE_F1)) {
        #if KBK_DEBUG_BUILD
            m_showCollisionDebug = !m_showCollisionDebug;
            KbkTrace(kLogChannel, "Collision debug %s", m_showCollisionDebug ? "ON" : "OFF");
        #endif
    }

    if (input.KeyPressed(SDL_SCANCODE_ESCAPE)) {
        m_menuVisible = !m_menuVisible;
    }

    m_time += dt;

    const float bobbing = std::sin(m_time * 2.0f) * 32.0f;
    const float sway = std::sin(m_time * 0.2f) * 300.0f;

    auto* centerEntity = m_scene.FindEntity(m_entityCenter);
    auto* rightEntity = m_scene.FindEntity(m_entityRight);

    if (centerEntity) {
        centerEntity->transform.position.x = 200.0f + sway;
        centerEntity->transform.position.y = 150.0f + bobbing;
        centerEntity->transform.rotation = m_time * 0.8f;
    }

    if (rightEntity) {
        rightEntity->transform.rotation = -m_time * 0.5f;
    }

    bool hit = false;
    if (centerEntity && rightEntity &&
        centerEntity->collision.circle && rightEntity->collision.circle) {

        hit = KibakoEngine::Intersects(*centerEntity->collision.circle, centerEntity->transform,
            *rightEntity->collision.circle, rightEntity->transform);
    }

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

    if (hit)
        KbkTrace(kLogChannel, "Center/Right COLLISION");

    m_lastCollision = hit;

    m_scene.Update(dt);

    UpdateUI(dt);
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

        const KibakoEngine::Color4 crossColor = KibakoEngine::Color4{ 1.0f, 1.0f, 0.4f, 1.0f };
        const KibakoEngine::Color4 aabbColor = KibakoEngine::Color4{ 0.9f, 0.9f, 0.2f, 1.0f };

        for (const KibakoEngine::Entity2D& entity : m_scene.Entities()) {
            if (!entity.active)
                continue;

            const KibakoEngine::Transform2D& transform = entity.transform;

            const bool drewCollider = KibakoEngine::DebugDraw2D::DrawCollisionComponent(batch,
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

    m_uiSystem.Render(batch);
}

void GameLayer::BuildUI()
{
    m_scoreLabel = nullptr;
    m_hintLabel = nullptr;
    m_hudScreen = nullptr;
    m_menuScreen = nullptr;
    m_uiSystem.Clear();

    if (!m_uiFont)
        return;

    auto hud = std::make_unique<KibakoEngine::UIScreen>();
    auto& hudRoot = hud->Root();

    auto& scoreLabel = hudRoot.EmplaceChild<KibakoEngine::UILabel>("HUD.Score");
    scoreLabel.SetFont(m_uiFont);
    scoreLabel.SetPosition({ 20.0f, 20.0f });
    scoreLabel.SetColor({ 1.0f, 0.95f, 0.4f, 1.0f });
    scoreLabel.SetScale(0.9f);

    auto& hintLabel = hudRoot.EmplaceChild<KibakoEngine::UILabel>("HUD.Hint");
    hintLabel.SetFont(m_uiFont);
    hintLabel.SetPosition({ 20.0f, 56.0f });
    hintLabel.SetColor({ 0.8f, 0.9f, 1.0f, 1.0f });
    hintLabel.SetScale(0.7f);
    hintLabel.SetText("Press ESC to toggle menu");

    m_scoreLabel = &scoreLabel;
    m_hintLabel = &hintLabel;
    m_hudScreen = hud.get();
    m_uiSystem.PushScreen(std::move(hud));

    auto menu = std::make_unique<KibakoEngine::UIScreen>();
    auto& menuRoot = menu->Root();

    auto& menuPanel = menuRoot.EmplaceChild<KibakoEngine::UIPanel>("Menu.Panel");
    menuPanel.SetSize({ 420.0f, 260.0f });
    menuPanel.SetAnchor(KibakoEngine::UIAnchor::Center);
    menuPanel.SetColor({ 0.08f, 0.09f, 0.13f, 0.94f });

    auto& titleLabel = menuPanel.EmplaceChild<KibakoEngine::UILabel>("Menu.Title");
    titleLabel.SetFont(m_uiFont);
    titleLabel.SetText("ASTRO VOID");
    titleLabel.SetPosition({ 24.0f, 24.0f });
    titleLabel.SetColor({ 1.0f, 0.96f, 0.65f, 1.0f });
    titleLabel.SetScale(1.1f);

    auto& playButton = menuPanel.EmplaceChild<KibakoEngine::UIButton>("Menu.Play");
    playButton.SetFont(m_uiFont);
    playButton.SetText("Play");
    playButton.SetSize({ 360.0f, 48.0f });
    playButton.SetPosition({ 30.0f, 100.0f });
    playButton.SetOnClick([this]() {
        m_menuVisible = false;
    });

    auto& quitButton = menuPanel.EmplaceChild<KibakoEngine::UIButton>("Menu.Quit");
    quitButton.SetFont(m_uiFont);
    quitButton.SetText("Quit");
    quitButton.SetSize({ 360.0f, 48.0f });
    quitButton.SetPosition({ 30.0f, 160.0f });
    quitButton.SetOnClick([]() {
        KbkLog(kLogChannel, "Quit clicked (hook your exit logic here)");
    });

    m_menuScreen = menu.get();
    m_uiSystem.PushScreen(std::move(menu));
}

void GameLayer::UpdateUI(float dt)
{
    m_uiSystem.SetScreenSize(static_cast<float>(m_app.Width()), static_cast<float>(m_app.Height()));

    if (m_scoreLabel) {
        char buffer[64]{};
        std::snprintf(buffer, sizeof(buffer), "Time: %.2f s", m_time);
        m_scoreLabel->SetText(buffer);
    }

    if (m_hintLabel)
        m_hintLabel->SetVisible(!m_menuVisible);

    if (m_hudScreen)
        m_hudScreen->SetVisible(true);

    if (m_menuScreen)
        m_menuScreen->SetVisible(m_menuVisible);

    m_uiSystem.Update(dt);
}
