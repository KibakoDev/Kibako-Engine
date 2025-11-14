#pragma once

#include <vector>

#include "KibakoEngine/Collision/Collision2D.h"
#include "KibakoEngine/Core/Layer.h"
#include "KibakoEngine/Renderer/SpriteTypes.h"
#include "KibakoEngine/Renderer/SpriteBatch2D.h"
#include "KibakoEngine/Scene/Scene2D.h"

namespace KibakoEngine {
    class Application;
    class Texture2D;
}

class GameLayer final : public KibakoEngine::Layer
{
public:
    explicit GameLayer(KibakoEngine::Application& app);

    void OnAttach() override;
    void OnDetach() override;
    void OnUpdate(float dt) override;
    void OnRender(KibakoEngine::SpriteBatch2D& batch) override;

    [[nodiscard]] KibakoEngine::Scene2D& Scene() { return m_scene; }
    [[nodiscard]] const KibakoEngine::Scene2D& Scene() const { return m_scene; }

    [[nodiscard]] std::vector<KibakoEngine::Entity2D>& Entities() { return m_scene.Entities(); }
    [[nodiscard]] const std::vector<KibakoEngine::Entity2D>& Entities() const { return m_scene.Entities(); }

private:
    KibakoEngine::Application& m_app;
    KibakoEngine::Texture2D* m_starTexture = nullptr;

    KibakoEngine::EntityID m_entityCenter = 0;
    KibakoEngine::EntityID m_entityRight = 0;

    KibakoEngine::CircleCollider2D m_centerCollider{};
    KibakoEngine::CircleCollider2D m_rightCollider{};

    bool  m_showCollisionDebug = false;
    bool  m_lastCollision = false;
    float m_time = 0.0f;

    KibakoEngine::Scene2D m_scene;
};
