// Kibako2DSandbox/src/GameLayer.cpp
#include "GameLayer.h"

#include "KibakoEngine/Core/Application.h"
#include "KibakoEngine/Core/Log.h"
#include "KibakoEngine/Core/Debug.h"
#include "KibakoEngine/Renderer/RendererD3D11.h"

#include <cmath>

using namespace KibakoEngine;

GameLayer::GameLayer(Application& app)
    : Layer("GameLayer")
    , m_app(app)
{
}

void GameLayer::OnAttach()
{
    auto* device = m_app.Renderer().GetDevice();
    KBK_ASSERT(device != nullptr, "Renderer device must exist before loading textures");

    if (!m_texStar.LoadFromFile(device, "assets/star.png", true)) {
        KbkLog("Sandbox", "Failed to load texture: assets/star.png");
    }
}

void GameLayer::OnDetach()
{
    // Rien de spécial : Texture2D se détruit tout seul (RAII)
}

void GameLayer::OnUpdate(float dt)
{
    // Juste un timer pour animer légèrement un sprite
    m_time += dt;
}

void GameLayer::OnRender(SpriteBatch2D& batch)
{
    if (!m_texStar.GetSRV()) {
        return;
    }

    const float w = static_cast<float>(m_texStar.Width());
    const float h = static_cast<float>(m_texStar.Height());

    // Petit bobbing vertical pour montrer que tout tourne
    const float xOffset = std::sin(m_time * 2.0f) * 200.0f;

    RectF dstCenter{ 200.0f + xOffset, 150.0f, w, h };
    RectF dstLeft{ 60.0f, 140.0f,           w, h };
    RectF dstRight{ 340.0f, 160.0f,           w, h };
    const RectF uvFull{ 0.0f, 0.0f, 1.0f, 1.0f };

    // layer -1 / 0 / +1 juste pour tester le z-order
    batch.Push(m_texStar, dstLeft, uvFull, Color4{ 0.2f, 0.8f, 1.0f, 1.0f }, 0.0f, -1);
    batch.Push(m_texStar, dstCenter, uvFull, Color4::White(), 0.0f, 0);
    batch.Push(m_texStar, dstRight, uvFull, Color4{ 1.0f, 0.5f, 0.3f, 1.0f }, 0.0f, 1);
}