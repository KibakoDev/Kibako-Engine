// GameLayer.cpp - Black & white sandbox demo for Kibako 2D Engine
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

    // Monochrome star texture
    m_starTexture = assets.LoadTexture("star", "assets/star.png", true);
    if (!m_starTexture || !m_starTexture->IsValid()) {
        KbkError(kLogChannel, "Failed to load texture: assets/star.png");
        return;
    }

    // UI font (monospace goes well with debug look)
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
    m_timeLabel = nullptr;
    m_entitiesLabel = nullptr;
    m_hintLabel = nullptr;
    m_statusLabel = nullptr;
    m_collisionLabel = nullptr;
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

    if (hit)
        KbkTrace(kLogChannel, "Center/Right collision");

    m_lastCollision = hit;

    m_scene.Update(dt);
}

// --------------------------------------------------------
// UI setup / update
// --------------------------------------------------------


void GameLayer::BuildUI()
{
    m_timeLabel = nullptr;
    m_entitiesLabel = nullptr;
    m_hintLabel = nullptr;
    m_statusLabel = nullptr;
    m_collisionLabel = nullptr;
    m_hudScreen = nullptr;
    m_menuScreen = nullptr;
    m_uiSystem.Clear();

    if (!m_uiFont)
        return;

    // Polished dark style with a bright accent
    m_accentColor = Color4{ 0.32f, 0.86f, 0.78f, 1.0f };
    m_warningColor = Color4{ 0.95f, 0.58f, 0.46f, 1.0f };
    m_mutedColor = Color4{ 0.72f, 0.78f, 0.86f, 1.0f };

    UIStyle style{};
    style.font = m_uiFont;
    style.headingColor = m_accentColor;
    style.primaryTextColor = Color4{ 0.92f, 0.95f, 1.0f, 1.0f };
    style.mutedTextColor = m_mutedColor;
    style.panelColor = Color4{ 0.05f, 0.07f, 0.10f, 0.94f };
    style.buttonNormal = Color4{ 0.12f, 0.14f, 0.18f, 0.95f };
    style.buttonHover = Color4{ 0.20f, 0.23f, 0.28f, 0.98f };
    style.buttonPressed = Color4{ 0.26f, 0.28f, 0.34f, 1.0f };
    style.buttonSize = DirectX::XMFLOAT2{ 360.0f, 48.0f };
    style.buttonPadding = DirectX::XMFLOAT2{ 18.0f, 12.0f };
    style.headingScale = 1.05f;
    style.bodyScale = 0.92f;
    style.captionScale = 0.78f;
    style.buttonTextScale = 0.96f;

    // ---------- HUD (top-left card) ----------
    auto hud = std::make_unique<UIScreen>();
    auto& hudRoot = hud->Root();

    auto& hudCard = hudRoot.EmplaceChild<UIPanel>("HUD.Card");
    hudCard.SetPosition({ 18.0f, 18.0f });
    hudCard.SetSize({ 440.0f, 178.0f });
    hudCard.SetColor(Color4{ 0.04f, 0.05f, 0.08f, 0.90f });

    auto& hudTitle = hudCard.EmplaceChild<UILabel>("HUD.Title");
    style.ApplyHeading(hudTitle);
    hudTitle.SetPosition({ 20.0f, 18.0f });
    hudTitle.SetScale(1.02f);
    hudTitle.SetText("KIBAKO SANDBOX");

    auto& hudSubtitle = hudCard.EmplaceChild<UILabel>("HUD.Subtitle");
    style.ApplyCaption(hudSubtitle);
    hudSubtitle.SetPosition({ 20.0f, 44.0f });
    hudSubtitle.SetColor(style.mutedTextColor);
    hudSubtitle.SetText("Polished playground for collisions and motion");

    auto& statusLabel = hudCard.EmplaceChild<UILabel>("HUD.Status");
    style.ApplyCaption(statusLabel);
    statusLabel.SetPosition({ 20.0f, 70.0f });
    statusLabel.SetColor(m_accentColor);
    statusLabel.SetText("RUNNING • realtime simulation");

    auto& timeLabel = hudCard.EmplaceChild<UILabel>("HUD.Time");
    style.ApplyBody(timeLabel);
    timeLabel.SetPosition({ 20.0f, 96.0f });
    timeLabel.SetText("Time  0.00 s");

    auto& collisionLabel = hudCard.EmplaceChild<UILabel>("HUD.Collision");
    style.ApplyBody(collisionLabel);
    collisionLabel.SetPosition({ 232.0f, 96.0f });
    collisionLabel.SetColor(style.mutedTextColor);
    collisionLabel.SetText("Collisions  none");

    auto& entitiesLabel = hudCard.EmplaceChild<UILabel>("HUD.Entities");
    style.ApplyBody(entitiesLabel);
    entitiesLabel.SetPosition({ 20.0f, 122.0f });
    entitiesLabel.SetText("Entities  0");

    auto& hintLabel = hudCard.EmplaceChild<UILabel>("HUD.Hint");
    style.ApplyCaption(hintLabel);
    hintLabel.SetPosition({ 20.0f, 148.0f });
    hintLabel.SetColor(style.mutedTextColor);
    hintLabel.SetText("F1 • Collision overlay      F3 • Command deck");

    m_statusLabel = &statusLabel;
    m_timeLabel = &timeLabel;
    m_collisionLabel = &collisionLabel;
    m_entitiesLabel = &entitiesLabel;
    m_hintLabel = &hintLabel;
    m_hudScreen = hud.get();

    m_uiSystem.PushScreen(std::move(hud));

    // ---------- Center menu (overlay) ----------
    auto menu = std::make_unique<UIScreen>();
    auto& root = menu->Root();

    auto& panel = root.EmplaceChild<UIPanel>("Menu.Panel");
    panel.SetSize({ 520.0f, 340.0f });
    panel.SetAnchor(UIAnchor::Center);
    panel.SetColor(Color4{ 0.05f, 0.07f, 0.11f, 0.96f });

    auto& hero = panel.EmplaceChild<UIPanel>("Menu.Hero");
    hero.SetSize({ 520.0f, 96.0f });
    hero.SetColor(Color4{ m_accentColor.r, m_accentColor.g, m_accentColor.b, 0.18f });

    auto& title = hero.EmplaceChild<UILabel>("Menu.Title");
    style.ApplyHeading(title);
    title.SetPosition({ 26.0f, 16.0f });
    title.SetColor(Color4::White());
    title.SetText("Sandbox command deck");

    auto& subtitle = hero.EmplaceChild<UILabel>("Menu.Subtitle");
    style.ApplyCaption(subtitle);
    subtitle.SetPosition({ 26.0f, 48.0f });
    subtitle.SetColor(Color4{ 0.90f, 0.94f, 1.0f, 1.0f });
    subtitle.SetText("Clean controls for a professional demo feel");

    auto& description = panel.EmplaceChild<UILabel>("Menu.Description");
    style.ApplyBody(description);
    description.SetPosition({ 26.0f, 116.0f });
    description.SetText("Pause the sandbox, toggle overlays, and keep the session tidy.");

    auto& notes = panel.EmplaceChild<UILabel>("Menu.Notes");
    style.ApplyCaption(notes);
    notes.SetPosition({ 26.0f, 140.0f });
    notes.SetColor(style.mutedTextColor);
    notes.SetText("This overlay keeps controls centered and legible on any resolution.");

    auto& resumeBtn = panel.EmplaceChild<UIButton>("Menu.Resume");
    resumeBtn.SetStyle(style);
    resumeBtn.SetPosition({ 26.0f, 188.0f });
    resumeBtn.SetText("Resume simulation");
    resumeBtn.SetOnClick([this]() {
        m_menuVisible = false;
        });

    auto& overlayBtn = panel.EmplaceChild<UIButton>("Menu.Overlay");
    overlayBtn.SetStyle(style);
    overlayBtn.SetPosition({ 26.0f, 240.0f });
    overlayBtn.SetText("Toggle collision overlay");
    overlayBtn.SetOnClick([this]() {
        m_showCollisionDebug = !m_showCollisionDebug;
        });

    auto& quitBtn = panel.EmplaceChild<UIButton>("Menu.Quit");
    quitBtn.SetStyle(style);
    quitBtn.SetPosition({ 26.0f, 292.0f });
    quitBtn.SetText("Exit sandbox (log only)");
    quitBtn.SetOnClick([]() {
        KbkLog(kLogChannel, "Quit Sandbox clicked (hook your exit logic here)");
        });

    m_menuScreen = menu.get();
    m_uiSystem.PushScreen(std::move(menu));
}

void GameLayer::UpdateUI(float dt)
{
    // Screen size can change (window resize)
    m_uiSystem.SetScreenSize(
        static_cast<float>(m_app.Width()),
        static_cast<float>(m_app.Height()));

    // TIME label
    if (m_timeLabel) {
        char buf[64]{};
        std::snprintf(buf, sizeof(buf), "Time   %.2f s", m_time);
        m_timeLabel->SetText(buf);
    }

    // ENTITIES label
    if (m_entitiesLabel) {
        char buf[64]{};
        const auto count = m_scene.Entities().size();
        std::snprintf(buf, sizeof(buf), "Entities   %zu",
            static_cast<std::size_t>(count));
        m_entitiesLabel->SetText(buf);
    }

    if (m_collisionLabel) {
        const char* state = m_lastCollision ? "Collisions   active" : "Collisions   none";
        m_collisionLabel->SetText(state);
        m_collisionLabel->SetColor(m_lastCollision ? m_warningColor : m_mutedColor);
    }

    if (m_statusLabel) {
        m_statusLabel->SetText(m_menuVisible
            ? "PAUSED • command deck open"
            : "RUNNING • realtime simulation");
        m_statusLabel->SetColor(m_menuVisible ? m_warningColor : m_accentColor);
    }

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
    const Color4 circleColorHit = Color4::White();
    const Color4 circleColorNormal = Color4{ 0.7f, 0.7f, 0.7f, 1.0f };
    const Color4 crossColor = Color4{ 1.0f, 1.0f, 1.0f, 1.0f };

    for (const Entity2D& e : m_scene.Entities()) {
        if (!e.active)
            continue;

        const Transform2D& t = e.transform;

        const Color4 circleColor = m_lastCollision ? circleColorHit : circleColorNormal;

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