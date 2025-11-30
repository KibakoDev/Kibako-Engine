// Sandbox layer for Kibako 2D Engine
#pragma once

#include <cstdint>

#include "KibakoEngine/Core/Layer.h"
#include "KibakoEngine/Renderer/SpriteTypes.h"
#include "KibakoEngine/Renderer/Texture2D.h"
#include "KibakoEngine/Scene/Scene2D.h"
#include "KibakoEngine/Collision/Collision2D.h"
#include "KibakoEngine/Fonts/Font.h"
#include "KibakoEngine/UI/UIElement.h"
#include "KibakoEngine/UI/UIControls.h"

namespace KibakoEngine {
    class Application;
}

class GameLayer final : public KibakoEngine::Layer
{
public:
    explicit GameLayer(KibakoEngine::Application& app);

    void OnAttach() override;
    void OnDetach() override;
    void OnUpdate(float dt) override;
    void OnRender(KibakoEngine::SpriteBatch2D& batch) override;

private:
    // Internal helpers
    void BuildUI();
    void UpdateUI(float dt);
    void UpdateScene(float dt);
    void RenderCollisionDebug(KibakoEngine::SpriteBatch2D& batch);

private:
    KibakoEngine::Application& m_app;

    // Gameplay
    KibakoEngine::Scene2D      m_scene;
    std::uint32_t              m_entityLeft = 0;
    std::uint32_t              m_entityRight = 0;
    KibakoEngine::CircleCollider2D m_leftCollider{};
    KibakoEngine::CircleCollider2D m_rightCollider{};

    KibakoEngine::Texture2D* m_starTexture = nullptr;
    const KibakoEngine::Font* m_uiFont = nullptr;

    // UI system
    KibakoEngine::UISystem   m_uiSystem;
    KibakoEngine::UILabel* m_titleLabel = nullptr;
    KibakoEngine::UILabel* m_timeLabel = nullptr;
    KibakoEngine::UILabel* m_stateLabel = nullptr;
    KibakoEngine::UILabel* m_entitiesLabel = nullptr;
    KibakoEngine::UIButton* m_resumeButton = nullptr;
    KibakoEngine::UIButton* m_collisionButton = nullptr;
    KibakoEngine::UIButton* m_exitButton = nullptr;
    KibakoEngine::UIScreen* m_hudScreen = nullptr;
    KibakoEngine::UIScreen* m_menuScreen = nullptr;

    bool  m_menuVisible = false;
    bool  m_showCollisionDebug = false;
    bool  m_lastCollision = false;
    float m_time = 0.0f;
};