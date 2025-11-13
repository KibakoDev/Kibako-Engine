#include "GameLayer.h"

#include "KibakoEngine/Core/Application.h"
#include "KibakoEngine/Core/Debug.h"
#include "KibakoEngine/Core/Log.h"
#include "KibakoEngine/Core/Profiler.h"
#include "KibakoEngine/Renderer/RendererD3D11.h"

#include <cmath>

using namespace KibakoEngine;

namespace
{
    constexpr const char* kLogChannel = "Sandbox";
}

GameLayer::GameLayer(Application& app)
    : Layer("Sandbox.GameLayer")
    , m_app(app)
{
    m_sprites = {
        SampleSprite{ RectF::FromXYWH(80.0f, 140.0f, 0.0f, 0.0f), Color4{ 0.25f, 0.8f, 1.0f, 1.0f }, 0.0f, -1 },
        SampleSprite{ RectF::FromXYWH(200.0f, 150.0f, 0.0f, 0.0f), Color4::White(), 0.8f, 0 },
        SampleSprite{ RectF::FromXYWH(340.0f, 160.0f, 0.0f, 0.0f), Color4{ 1.0f, 0.55f, 0.35f, 1.0f }, -0.5f, 1 },
    };
}

void GameLayer::OnAttach()
{
    KBK_PROFILE_SCOPE("GameLayerAttach");

    auto& assets = m_app.Assets();

    m_starTexture = assets.LoadTexture("star", "assets/star.png", true);
    if (!m_starTexture) {
        KbkError(kLogChannel, "Failed to load texture: assets/star.png");
        return;
    }

    const float width = static_cast<float>(m_starTexture->Width());
    const float height = static_cast<float>(m_starTexture->Height());

    for (auto& sprite : m_sprites) {
        sprite.baseRect.w = width;
        sprite.baseRect.h = height;
    }

    if (m_starTexture && m_starTexture->IsValid())
    {
        KbkLog(kLogChannel,
            "GameLayer attached (%d x %d texture)",
            m_starTexture->Width(),
            m_starTexture->Height());
    }
}

void GameLayer::OnDetach()
{
    KBK_PROFILE_SCOPE("GameLayerDetach");
    m_starTexture = nullptr;
}

void GameLayer::OnUpdate(float dt)
{
    KBK_PROFILE_SCOPE("GameLayerUpdate");
    m_time += dt;
}

void GameLayer::OnRender(SpriteBatch2D& batch)
{
    KBK_PROFILE_SCOPE("GameLayerRender");

    if (!m_starTexture || !m_starTexture->IsValid())
        return;

    const RectF uvFull{ 0.0f, 0.0f, 1.0f, 1.0f };
    const float bobbing = std::sin(m_time * 2.0f) * 32.0f;

    for (std::size_t i = 0; i < m_sprites.size(); ++i) {
        const SampleSprite& sprite = m_sprites[i];

        RectF dst = sprite.baseRect;
        if (i == 1) {
            dst.x += std::sin(m_time * 1.5f) * 200.0f;
            dst.y += bobbing;
        }

        const float rotation = sprite.rotationSpeed != 0.0f ? m_time * sprite.rotationSpeed : 0.0f;
        batch.Push(*m_starTexture, dst, uvFull, sprite.color, rotation, sprite.layer);
    }
}

