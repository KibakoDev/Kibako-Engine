#pragma once

#include "KibakoEngine/Core/Layer.h"
#include "KibakoEngine/Renderer/SpriteTypes.h"
#include "KibakoEngine/Renderer/Texture2D.h"
#include "KibakoEngine/Scene/Scene2D.h"
#include "KibakoEngine/Collision/Collision2D.h"

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
    KibakoEngine::Application& m_app;

    KibakoEngine::Texture2D* m_starTexture = nullptr;
    KibakoEngine::Texture2D  m_debugPixel;
    KibakoEngine::Scene2D    m_scene;

    // Entity identifiers used during gameplay updates
    KibakoEngine::EntityID m_entityLeft = 0;
    KibakoEngine::EntityID m_entityCenter = 0;
    KibakoEngine::EntityID m_entityRight = 0;

    // Colliders owned by the layer (entities keep raw pointers)
    KibakoEngine::CircleCollider2D m_centerCollider{};
    KibakoEngine::CircleCollider2D m_rightCollider{};

    float m_time = 0.0f;
    bool  m_showCollisionDebug = false;
    bool  m_lastCollision = false;
};
