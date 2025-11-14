#pragma once

#include "KibakoEngine/Core/Layer.h"
#include "KibakoEngine/Renderer/SpriteTypes.h"
#include "KibakoEngine/Renderer/Texture2D.h"
#include "KibakoEngine/Scene/Scene2D.h"

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

    KibakoEngine::Scene2D m_scene;

    KibakoEngine::EntityID m_entityCenter = 0;
    KibakoEngine::EntityID m_entityBobbing = 0;
    KibakoEngine::EntityID m_entityRight = 0;

    float m_time = 0.0f;
};