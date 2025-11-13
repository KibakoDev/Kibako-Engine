// Kibako2DSandbox/include/GameLayer.h
#pragma once

#include "KibakoEngine/Core/Layer.h"
#include "KibakoEngine/Renderer/Texture2D.h"
#include "KibakoEngine/Renderer/SpriteTypes.h"
#include "KibakoEngine/Renderer/SpriteBatch2D.h"

namespace KibakoEngine {
    class Application;
}

class GameLayer : public KibakoEngine::Layer
{
public:
    explicit GameLayer(KibakoEngine::Application& app);

    void OnAttach() override;
    void OnDetach() override;
    void OnUpdate(float dt) override;
    void OnRender(KibakoEngine::SpriteBatch2D& batch) override;

private:
    KibakoEngine::Application& m_app;

    KibakoEngine::Texture2D m_texStar{};
    float m_time = 0.0f;
};