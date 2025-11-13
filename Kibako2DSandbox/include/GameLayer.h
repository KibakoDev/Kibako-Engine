#pragma once

#include <array>

#include "KibakoEngine/Core/Layer.h"
#include "KibakoEngine/Renderer/SpriteTypes.h"
#include "KibakoEngine/Renderer/Texture2D.h"

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
    struct SampleSprite
    {
        KibakoEngine::RectF baseRect;
        KibakoEngine::Color4 color;
        float rotationSpeed = 0.0f;
        int layer = 0;
    };

    KibakoEngine::Application& m_app;
    KibakoEngine::Texture2D m_starTexture{};
    std::array<SampleSprite, 3> m_sprites{};
    float m_time = 0.0f;
};

