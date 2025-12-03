// Sandbox gameplay layer
#include "GameLayer.h"

#include "KibakoEngine/Core/Application.h"
#include "KibakoEngine/Core/Debug.h"
#include "KibakoEngine/Core/DebugUI.h"
#include "KibakoEngine/Core/Log.h"
#include "KibakoEngine/Core/Profiler.h"
#include "KibakoEngine/Renderer/DebugDraw2D.h"

#include <DirectXMath.h>
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_scancode.h>

#include <algorithm>
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

    struct SandboxMenuTheme
    {
        Color4 background{ 0.0f, 0.0f, 0.0f, 0.78f };
        Color4 panelColor{ 0.05f, 0.05f, 0.05f, 0.88f };
        Color4 buttonNormal{ 1.0f, 1.0f, 1.0f, 1.0f };
        Color4 buttonHover{ 0.88f, 0.88f, 0.88f, 1.0f };
        Color4 buttonPressed{ 0.78f, 0.78f, 0.78f, 1.0f };
        Color4 muted{ 0.75f, 0.75f, 0.75f, 1.0f };

        DirectX::XMFLOAT2 buttonSize{ 440.0f, 64.0f };
        DirectX::XMFLOAT2 buttonPadding{ 48.0f, 14.0f };

        float titleScale = 0.95f;
        float bodyScale = 0.42f;
        float buttonTextScale = 0.46f;
        float verticalSpacing = 28.0f;
        float titleSpacing = 54.0f;
        float infoPanelHeight = 124.0f;
    };

    const SandboxMenuTheme kMenuTheme{};

#if KBK_DEBUG_BUILD
    // Basic ImGui scene inspector
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

GameLayer::GameLayer(Application& app)
    : Layer("Sandbox.GameLayer")
    , m_app(app)
{
}

void GameLayer::OnAttach()
{
    KBK_PROFILE_SCOPE("GameLayerAttach");

    auto& assets = m_app.Assets();

    // Load texture
    m_starTexture = assets.LoadTexture("star", "assets/star.png", true);
    if (!m_starTexture || !m_starTexture->IsValid()) {
        KbkError(kLogChannel, "Failed to load texture: assets/star.png");
        return;
    }

    // Load font
    m_uiFont = assets.LoadFontTTF("ui.default", "assets/fonts/dogica.ttf", 32);
    if (!m_uiFont)
        KbkWarn(kLogChannel, "Failed to load font: assets/fonts/dogica.ttf");

    // Scene setup
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

    // Left star
    {
        Entity2D& e = m_scene.CreateEntity();
        m_entityLeft = e.id;

        configureSprite(e,
            { 530.0f, 350.0f },
            { 1.2f, 1.2f },
            Color4::White(),
            0);

        m_leftCollider.radius = 0.5f * texW * e.transform.scale.x;
        m_leftCollider.active = true;
        e.collision.circle = &m_leftCollider;
    }

    // Right star
    {
        Entity2D& e = m_scene.CreateEntity();
        m_entityRight = e.id;

        configureSprite(e,
            { 700.0f, 350.0f },
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

    // UI system
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

    m_entityLeft = 0;
    m_entityRight = 0;

    m_leftCollider = {};
    m_rightCollider = {};

    m_showCollisionDebug = false;
    m_lastCollision = false;
    m_menuVisible = true;
    m_time = 0.0f;

    m_uiSystem.Clear();
    m_titleLabel = nullptr;
    m_timeLabel = nullptr;
    m_stateLabel = nullptr;
    m_entitiesLabel = nullptr;
    m_resumeButton = nullptr;
    m_exitButton = nullptr;
    m_hudScreen = nullptr;
    m_menuScreen = nullptr;
    m_menuBackdrop = nullptr;

#if KBK_DEBUG_BUILD
    DebugUI::SetSceneInspector(nullptr, nullptr);
#endif
}

void GameLayer::OnUpdate(float dt)
{
    KBK_PROFILE_SCOPE("GameLayerUpdate");

    auto& input = m_app.InputSys();

    if (input.KeyPressed(SDL_SCANCODE_F1))
        m_showCollisionDebug = !m_showCollisionDebug;

    if (input.KeyPressed(SDL_SCANCODE_F3))
        m_menuVisible = !m_menuVisible;

    if (input.KeyPressed(SDL_SCANCODE_ESCAPE)) {
        SDL_Event quit{};
        quit.type = SDL_QUIT;
        SDL_PushEvent(&quit);
    }

    // Pause when menu is open
    if (!m_menuVisible)
        UpdateScene(dt);

    UpdateUI(dt);
}

void GameLayer::OnRender(SpriteBatch2D& batch)
{
    KBK_PROFILE_SCOPE("GameLayerRender");

    if (!m_starTexture || !m_starTexture->IsValid())
        return;

    m_scene.Render(batch);

    if (m_showCollisionDebug)
        RenderCollisionDebug(batch);

    m_uiSystem.Render(batch);
}

void GameLayer::UpdateScene(float dt)
{
    m_time += dt;

    Entity2D* left = m_scene.FindEntity(m_entityLeft);
    Entity2D* right = m_scene.FindEntity(m_entityRight);

    // Left star motion
    if (left) {
        left->transform.rotation = m_time * 0.7f;
    }

    // Right star motion
    if (right) {
        right->transform.rotation = -m_time * 0.5f;
    }

    bool hit = false;
    if (left && right &&
        left->collision.circle && right->collision.circle) {

        hit = Intersects(*left->collision.circle, left->transform,
            *right->collision.circle, right->transform);
    }

    // Collision feedback
    if (left) {
        left->sprite.color = hit
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

void GameLayer::BuildUI()
{
    m_titleLabel = nullptr;
    m_timeLabel = nullptr;
    m_stateLabel = nullptr;
    m_entitiesLabel = nullptr;
    m_resumeButton = nullptr;
    m_exitButton = nullptr;
    m_hudScreen = nullptr;
    m_menuScreen = nullptr;
    m_uiSystem.Clear();

    if (!m_uiFont)
        return;

    auto& style = m_uiSystem.Style();
    style.font = m_uiFont;
    style.headingColor = Color4::White();
    style.primaryTextColor = Color4::Black();
    style.mutedTextColor = kMenuTheme.muted;
    style.panelColor = kMenuTheme.panelColor;
    style.buttonNormal = kMenuTheme.buttonNormal;
    style.buttonHover = kMenuTheme.buttonHover;
    style.buttonPressed = kMenuTheme.buttonPressed;
    style.buttonSize = kMenuTheme.buttonSize;
    style.buttonPadding = kMenuTheme.buttonPadding;
    style.headingScale = kMenuTheme.titleScale;
    style.bodyScale = kMenuTheme.bodyScale;
    style.buttonTextScale = kMenuTheme.buttonTextScale;

    const float headingHeight = TextRenderer::MeasureText(*style.font, "S", style.headingScale).lineHeight;
    const float bodyHeight = TextRenderer::MeasureText(*style.font, "S", style.bodyScale).lineHeight;
    const float lineSpacing = 6.0f;

    // HUD layout
    UIScreen& hud = m_uiSystem.CreateScreen("HUD");
    auto& hudRoot = hud.Root();

    auto& hudGroup = hudRoot.EmplaceChild<UIElement>("HUD.Group");
    hudGroup.SetPosition({ 16.0f, 16.0f });

    float hudY = 0.0f;
    auto makeHudLabel = [&](const char* name, float height, auto styler, const char* text) -> UILabel& {
        auto& lbl = hudGroup.EmplaceChild<UILabel>(name);
        styler(lbl);
        lbl.SetPosition({ 0.0f, hudY });
        lbl.SetText(text);
        hudY += height + lineSpacing;
        return lbl;
    };

    m_titleLabel = &makeHudLabel("HUD.Title", headingHeight + 5, [&](UILabel& lbl) { style.ApplyHeading(lbl); }, "KIBAKO 2D ENGINE - SANDBOX");
    m_timeLabel = &makeHudLabel("HUD.Time", bodyHeight, [&](UILabel& lbl) { style.ApplyBody(lbl); }, "TIME  0.00 s");
    m_stateLabel = &makeHudLabel("HUD.State", bodyHeight, [&](UILabel& lbl) { style.ApplyBody(lbl); }, "COLLISION  IDLE");
    m_entitiesLabel = &makeHudLabel("HUD.Entities", bodyHeight, [&](UILabel& lbl) { style.ApplyBody(lbl); }, "ENTITIES  0");

    auto& hintLabel = hudGroup.EmplaceChild<UILabel>("HUD.Hint");
    style.ApplyCaption(hintLabel);
    hintLabel.SetPosition({ 0.0f, hudY });
    hintLabel.SetText("F3: Menu  ·  F1: Toggle collisions  ·  ESC: Quit");

    m_hudScreen = &hud;

    // Menu layout
    UIScreen& menu = m_uiSystem.CreateScreen("Menu");
    auto& menuRoot = menu.Root();

    auto& dim = menuRoot.EmplaceChild<UIPanel>("Menu.Backdrop");
    dim.SetColor(kMenuTheme.background);
    dim.SetAnchor(UIAnchor::TopLeft);
    dim.SetSize({ static_cast<float>(m_app.Width()), static_cast<float>(m_app.Height()) });
    m_menuBackdrop = &dim;

    auto& stack = menuRoot.EmplaceChild<UIElement>("Menu.Stack");
    stack.SetAnchor(UIAnchor::Center);

    float yOffset = -kMenuTheme.titleSpacing;

    auto& title = stack.EmplaceChild<UILabel>("Menu.Title");
    style.ApplyHeading(title);
    title.SetAnchor(UIAnchor::Center);
    title.SetScale(kMenuTheme.titleScale + 0.25f);
    title.SetText("ASTRO VOID");
    title.SetPosition({ 0.0f, yOffset });
    yOffset += kMenuTheme.titleSpacing;

    auto makeMenuButton = [&](const char* name, const char* text, auto onClick) -> UIButton& {
        auto& btn = stack.EmplaceChild<UIButton>(name);
        style.ApplyButton(btn);
        btn.SetAnchor(UIAnchor::Center);
        btn.SetPosition({ 0.0f, yOffset });
        btn.SetText(text);
        btn.SetOnClick(onClick);
        yOffset += kMenuTheme.buttonSize.y + kMenuTheme.verticalSpacing;
        return btn;
    };

    m_resumeButton = &makeMenuButton("Menu.Resume", "NEW GAME", [this]() {
        m_time = 0.0f;
        m_menuVisible = false;
    });

    m_exitButton = &makeMenuButton("Menu.Exit", "EXIT", []() {
        SDL_Event quit{};
        quit.type = SDL_QUIT;
        SDL_PushEvent(&quit);
    });

    menu.SetVisible(true);
    m_menuScreen = &menu;
}

void GameLayer::UpdateUI(float dt)
{
    (void)dt;

    // Resize-aware
    const float screenW = static_cast<float>(m_app.Width());
    const float screenH = static_cast<float>(m_app.Height());

    m_uiSystem.SetScreenSize(screenW, screenH);

    if (m_menuBackdrop)
        m_menuBackdrop->SetSize({ screenW, screenH });

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

    // Title text
    if (m_titleLabel) {
        m_titleLabel->SetText(m_menuVisible
            ? "KIBAKO 2D ENGINE - SANDBOX  ·  PAUSED"
            : "KIBAKO 2D ENGINE - SANDBOX");
    }

    // HUD always visible

    if (m_hudScreen)
        m_hudScreen->SetVisible(true);

    if (m_menuScreen)
        m_menuScreen->SetVisible(m_menuVisible);

    if (m_resumeButton) {
        m_resumeButton->SetText(m_time > 0.01f ? "RESUME" : "NEW GAME");
    }

    m_uiSystem.Update(dt);
}

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
