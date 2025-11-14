#pragma once

#include <array>
#include <vector>

#include "KibakoEngine/Core/Layer.h"
#include "KibakoEngine/Renderer/SpriteTypes.h"
#include "KibakoEngine/Renderer/Texture2D.h"
#include "KibakoEngine/Renderer/SpriteBatch2D.h"
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

    // Accès pour le panel ImGui (Scene / Entities)
    KibakoEngine::Scene2D& Scene() { return m_scene; }
    const KibakoEngine::Scene2D& Scene() const { return m_scene; }

    std::vector<KibakoEngine::Entity2D>& Entities() { return m_entities; }
    const std::vector<KibakoEngine::Entity2D>& Entities() const { return m_entities; }

private:
    struct SampleSprite
    {
        KibakoEngine::RectF  baseRect;
        KibakoEngine::Color4 color;
        float                rotationSpeed = 0.0f;
        int                  layer = 0;
    };

    KibakoEngine::Application& m_app;
    KibakoEngine::Texture2D* m_starTexture = nullptr;
    std::array<SampleSprite, 3>         m_sprites{};
    float                               m_time = 0.0f;

    KibakoEngine::Scene2D               m_scene;
    std::vector<KibakoEngine::Entity2D> m_entities;
};