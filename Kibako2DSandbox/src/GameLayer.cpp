// GameLayer.cpp - Minimal black & white sandbox demo for Kibako 2D Engine
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

#if KBK_DEBUG_BUILD
#    include "imgui.h"
#endif

using namespace KibakoEngine;

namespace
{
    constexpr const char* kLogChannel = "Sandbox";
    constexpr int         kDebugDrawLayer = 1000;
    constexpr float       kColliderThickness = 2.0f;

#if KBK_DEBUG_BUILD
    // Simple ImGui scene inspector (debug only)
    void SceneInspectorPanel(void* userData)
    {
        auto* scene = static_cast<Scene2D*>(userData);
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
                const Entity2D& e = entities[static_cast<std::size_t>(i)];

                char label[64];
                std::snprintf(label, sizeof(label), "ID %u%s",
                    e.id,
                    e.active ? "" : " (disabled)");

                bool isSelected = (selectedIndex == i);
                if (ImGui::Selectable(label, isSelected))
                    selectedIndex = i;
                if (isSelected)
                    ImGui::SetItemDefaultFocus();
            }
            ImGui::EndListBox();
        }

        ImGui::Separator();

        if (selectedIndex >= 0 && selectedIndex < static_cast<int>(entities.size())) {
            Entity2D& e = entities[static_cast<std::size_t>(selectedIndex)];

            ImGui::Text("Selected ID: %u", e.id);
            ImGui::Checkbox("Active", &e.active);

            Transform2D& t = e.transform;
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
#endif

} // namespace

// --------------------------------------------------------
// Construction / lifecycle
// --------------------------------------------------------

GameLayer::GameLayer(Application& app)
    : Layer("Sandbox.GameLayer")
    , m_app(app)
{
}

void GameLayer::OnAttach()
{
    KBK_PROFILE_SCOPE("GameLayerAttach");

    auto& assets = m_app.Assets();

    // Star texture (already monochrome in your asset)
    m_starTexture = assets.LoadTexture("star", "assets/star.png", true);
    if (!m_starTexture || !m_starTexture->IsValid()) {
        KbkError(kLogChannel, "Failed to load texture: assets/star.png");
        return;
    }

    // UI font
    m_uiFont = assets.LoadFontTTF("ui.default", "assets/fonts/RobotoMono-Regular.ttf", 32);
    if (!m_uiFont)
        KbkWarn(kLogChannel, "Failed to load font: assets/fonts/RobotoMono-Regular.ttf");

    // ---------- Scene setup (3 stars, B&W) ----------
    const float texW = static_cast<float>(m_starTexture->Width());
    const float texH = static_cast<float>(m_starTexture->Height());
    const RectF spriteRect = RectF::FromXYWH(0.0f, 0.0f, texW, texH);
    const RectF uvRect = RectF::FromXYWH(0.0f, 0.0f, 1.0f, 1.0f);

    auto configureSprite = [&](Entity2D& e,
        const DirectX::XMFLOAT2& pos,
        const DirectX::XMFLOAT2& scale,
        const Color4& color,
        int layer)
        {
            e.transform.position = pos;
            e.transform.rotation = 0.0f;
            e.transform.scale = scale;

            e.sprite.texture = m_starTexture;
            e.sprite.dst = spriteRect;
            e.sprite.src = uvRect;
            e.sprite.color = color;
            e.sprite.layer = layer;
        };

    // Left star – light grey
    {
        Entity2D& e = m_scene.CreateEntity();
        configureSprite(e,
            { 80.0f, 140.0f },
            { 1.0f, 1.0f },
            Color4{ 0.7f, 0.7f, 0.7f, 1.0f },
            -1);
    }

    // Center star – white, with circle collider
    {
        Entity2D& e = m_scene.CreateEntity();
        m_entityCenter = e.id;

        configureSprite(e,
            { 220.0f, 150.0f },
            { 1.2f, 1.2f },
            Color4::White(),
            0);

        m_centerCollider.radius = 0.5f * texW * e.transform.scale.x;
        m_centerCollider.active = true;
        e.collision.circle = &m_centerCollider;
    }

    // Right star – mid grey, with circle collider
    {
        Entity2D& e = m_scene.CreateEntity();
        m_entityRight = e.id;

        configureSprite(e,
            { 360.0f, 160.0f },
            { 1.0f, 1.0f },
            Color4{ 0.55f, 0.55f, 0.55f, 1.0f },
            1);

        m_rightCollider.radius = 0.5f * texW * e.transform.scale.x;
        m_rightCollider.active = true;
        e.collision.circle = &m_rightCollider;
    }

    KbkLog(kLogChannel,
        "GameLayer attached (%d x %d texture, %zu entities)",
        m_starTexture->Width(),
        m_starTexture->Height(),
        m_scene.Entities().size());

    // UI system setup
    m_uiSystem.SetInput(&m_app.InputSys());
    BuildUI();

#if KBK_DEBUG_BUILD
    DebugUI::SetSceneInspector(&m_scene, &SceneInspectorPanel);
#endif
}

void GameLayer::OnDetach()
{
    KBK_PROFILE_SCOPE("GameLayerDetach");

    m_starTexture = nullptr;
    m_uiFont = nullptr;

    m_scene.Clear();

    m_entityCenter = 0;
    m_entityRight = 0;

    m_centerCollider = {};
    m_rightCollider = {};

    m_showCollisionDebug = false;
    m_lastCollision = false;
    m_menuVisible = false;
    m_time = 0.0f;

    m_uiSystem.Clear();
    m_titleLabel = nullptr;
    m_timeLabel = nullptr;
    m_stateLabel = nullptr;
    m_entitiesLabel = nullptr;
    m_hintLabel = nullptr;
    m_hudScreen = nullptr;
    m_menuScreen = nullptr;

#if KBK_DEBUG_BUILD
    DebugUI::SetSceneInspector(nullptr, nullptr);
#endif
}

// --------------------------------------------------------
// Update / Render
// --------------------------------------------------------

void GameLayer::OnUpdate(float dt)
{
    KBK_PROFILE_SCOPE("GameLayerUpdate");

    auto& input = m_app.InputSys();

    if (input.KeyPressed(SDL_SCANCODE_F1))
        m_showCollisionDebug = !m_showCollisionDebug;

    if (input.KeyPressed(SDL_SCANCODE_F3))
        m_menuVisible = !m_menuVisible;

    // Pause gameplay when menu is open
    if (!m_menuVisible)
        UpdateScene(dt);

    UpdateUI(dt);
}

void GameLayer::OnRender(SpriteBatch2D& batch)
{
    KBK_PROFILE_SCOPE("GameLayerRender");

    if (!m_starTexture || !m_starTexture->IsValid())
        return;

    // Sprites
    m_scene.Render(batch);

    // Collision overlay
    if (m_showCollisionDebug)
        RenderCollisionDebug(batch);

    // HUD + menu
    m_uiSystem.Render(batch);
}

// --------------------------------------------------------
// Scene logic
// --------------------------------------------------------

void GameLayer::UpdateScene(float dt)
{
    m_time += dt;

    const float bobbing = std::sin(m_time * 2.0f) * 32.0f;
    const float sway = std::sin(m_time * 0.25f) * 260.0f;

    Entity2D* center = m_scene.FindEntity(m_entityCenter);
    Entity2D* right = m_scene.FindEntity(m_entityRight);

    // Center star – movement + rotation
    if (center) {
        center->transform.position.x = 220.0f + sway;
        center->transform.position.y = 150.0f + bobbing;
        center->transform.rotation = m_time * 0.7f;
    }

    // Right star – counter rotation
    if (right) {
        right->transform.rotation = -m_time * 0.5f;
    }

    bool hit = false;
    if (center && right &&
        center->collision.circle && right->collision.circle) {

        hit = Intersects(*center->collision.circle, center->transform,
            *right->collision.circle, right->transform);
    }

    // Monochrome visual feedback
    if (center) {
        center->sprite.color = hit
            ? Color4::White()
            : Color4{ 0.9f, 0.9f, 0.9f, 1.0f };
    }

    if (right) {
        right->sprite.color = hit
            ? Color4{ 0.85f, 0.85f, 0.85f, 1.0f }
        : Color4{ 0.55f, 0.55f, 0.55f, 1.0f };
    }

    m_lastCollision = hit;
    m_scene.Update(dt);
}

// --------------------------------------------------------
// UI setup / update
// --------------------------------------------------------

void GameLayer::BuildUI()
{
    m_titleLabel = nullptr;
    m_timeLabel = nullptr;
    m_stateLabel = nullptr;
    m_entitiesLabel = nullptr;
    m_hintLabel = nullptr;
    m_hudScreen = nullptr;
    m_menuScreen = nullptr;
    m_uiSystem.Clear();

    if (!m_uiFont)
        return;

    // -------- Global monochrome style --------
    UIStyle style{};
    style.font = m_uiFont;
    style.headingColor = Color4::White();
    style.primaryTextColor = Color4::White();
    style.mutedTextColor = Color4{ 0.55f, 0.55f, 0.55f, 1.0f };
    style.headingScale = 0.90f;
    style.bodyScale = 0.85f;
    style.captionScale = 0.70f;

    // ===================================================
    // HUD TOP-LEFT (texte uniquement)
    // ===================================================
    auto hud = std::make_unique<UIScreen>();
    auto& hudRoot = hud->Root();

    auto& title = hudRoot.EmplaceChild<UILabel>("HUD.Title");
    style.ApplyHeading(title);
    title.SetPosition({ 16.0f, 16.0f });
    title.SetText("KIBAKO SANDBOX");

    auto& timeLabel = hudRoot.EmplaceChild<UILabel>("HUD.Time");
    style.ApplyBody(timeLabel);
    timeLabel.SetPosition({ 16.0f, 40.0f });
    timeLabel.SetText("TIME  0.00 s");

    auto& stateLabel = hudRoot.EmplaceChild<UILabel>("HUD.State");
    style.ApplyBody(stateLabel);
    stateLabel.SetPosition({ 16.0f, 62.0f });
    stateLabel.SetText("COLLISION  IDLE");

    auto& entitiesLabel = hudRoot.EmplaceChild<UILabel>("HUD.Entities");
    style.ApplyBody(entitiesLabel);
    entitiesLabel.SetPosition({ 16.0f, 84.0f });
    entitiesLabel.SetText("ENTITIES  0");

    auto& hintLabel = hudRoot.EmplaceChild<UILabel>("HUD.Hint");
    style.ApplyCaption(hintLabel);
    hintLabel.SetPosition({ 16.0f, 108.0f });
    hintLabel.SetColor(style.mutedTextColor);
    hintLabel.SetText("F1  collision overlay    ·    F3  command palette");

    m_titleLabel = &title;
    m_timeLabel = &timeLabel;
    m_stateLabel = &stateLabel;
    m_entitiesLabel = &entitiesLabel;
    m_hintLabel = &hintLabel;
    m_hudScreen = hud.get();

    m_uiSystem.PushScreen(std::move(hud));

    // ===================================================
    // CENTER COMMAND PALETTE (TEXTE UNIQUEMENT, PAS DE PANEL)
    // ===================================================
    auto menu = std::make_unique<UIScreen>();
    auto& root = menu->Root();

    // On calcule un centre approximatif à partir de la fenêtre actuelle
    const float cx = 0.5f * static_cast<float>(m_app.Width());
    const float cy = 0.5f * static_cast<float>(m_app.Height());
    const float lineH = 24.0f;
    const float left = cx - 180.0f;    // léger offset vers la gauche

    auto& menuTitle = root.EmplaceChild<UILabel>("Menu.Title");
    style.ApplyHeading(menuTitle);
    menuTitle.SetPosition({ left, cy - lineH * 2.0f });
    menuTitle.SetText("COMMANDS");

    auto& line1 = root.EmplaceChild<UILabel>("Menu.Line1");
    style.ApplyBody(line1);
    line1.SetPosition({ left, cy - lineH * 0.5f });
    line1.SetText("F3  Resume sandbox");

    auto& line2 = root.EmplaceChild<UILabel>("Menu.Line2");
    style.ApplyBody(line2);
    line2.SetPosition({ left, cy + lineH * 0.5f });
    line2.SetText("F1  Toggle collision overlay");

    auto& line3 = root.EmplaceChild<UILabel>("Menu.Line3");
    style.ApplyBody(line3);
    line3.SetPosition({ left, cy + lineH * 1.5f });
    line3.SetText("ESC  Exit sandbox");

    m_menuScreen = menu.get();
    m_uiSystem.PushScreen(std::move(menu));
}

void GameLayer::UpdateUI(float dt)
{
    (void)dt;

    // Resize-aware
    m_uiSystem.SetScreenSize(
        static_cast<float>(m_app.Width()),
        static_cast<float>(m_app.Height()));

    // Time
    if (m_timeLabel) {
        char buf[64]{};
        std::snprintf(buf, sizeof(buf), "TIME  %.2f s", m_time);
        m_timeLabel->SetText(buf);
    }

    // Entities
    if (m_entitiesLabel) {
        char buf[64]{};
        const auto count = m_scene.Entities().size();
        std::snprintf(buf, sizeof(buf), "ENTITIES  %zu",
            static_cast<std::size_t>(count));
        m_entitiesLabel->SetText(buf);
    }

    // Collision state
    if (m_stateLabel) {
        m_stateLabel->SetText(m_lastCollision
            ? "COLLISION  ACTIVE"
            : "COLLISION  IDLE");
    }

    // Title hint when paused
    if (m_titleLabel) {
        m_titleLabel->SetText(m_menuVisible
            ? "KIBAKO SANDBOX  ·  PAUSED"
            : "KIBAKO SANDBOX");
    }

    // HUD always visible, hint hidden when menu open
    if (m_hintLabel)
        m_hintLabel->SetVisible(!m_menuVisible);

    if (m_hudScreen)
        m_hudScreen->SetVisible(true);

    if (m_menuScreen)
        m_menuScreen->SetVisible(m_menuVisible);

    m_uiSystem.Update(dt);
}

// --------------------------------------------------------
// Collision debug rendering
// --------------------------------------------------------

void GameLayer::RenderCollisionDebug(SpriteBatch2D& batch)
{
    const Color4 circleHit = Color4::White();
    const Color4 circleIdle = Color4{ 0.7f, 0.7f, 0.7f, 1.0f };
    const Color4 crossColor = Color4::White();

    for (const Entity2D& e : m_scene.Entities()) {
        if (!e.active)
            continue;

        const Transform2D& t = e.transform;
        const Color4 circleColor = m_lastCollision ? circleHit : circleIdle;

        const bool drew = DebugDraw2D::DrawCollisionComponent(batch,
            t,
            e.collision,
            circleColor,
            circleColor,
            kColliderThickness,
            kDebugDrawLayer,
            48);

        if (drew) {
            DebugDraw2D::DrawCross(batch,
                t.position,
                10.0f,
                crossColor,
                kColliderThickness,
                kDebugDrawLayer);
        }
    }
}