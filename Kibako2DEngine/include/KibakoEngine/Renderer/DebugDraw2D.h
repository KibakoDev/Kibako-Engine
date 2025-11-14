#pragma once

#include <DirectXMath.h>

#include "KibakoEngine/Renderer/SpriteBatch2D.h"
#include "KibakoEngine/Renderer/SpriteTypes.h"

namespace KibakoEngine::DebugDraw2D {

    void DrawLine(SpriteBatch2D& batch,
        const DirectX::XMFLOAT2& a,
        const DirectX::XMFLOAT2& b,
        const Color4& color,
        float thickness = 1.0f,
        int layer = 0);

    void DrawCross(SpriteBatch2D& batch,
        const DirectX::XMFLOAT2& center,
        float size,
        const Color4& color,
        float thickness = 1.0f,
        int layer = 0);

    void DrawCircleOutline(SpriteBatch2D& batch,
        const DirectX::XMFLOAT2& center,
        float radius,
        const Color4& color,
        float thickness = 1.0f,
        int layer = 0,
        int segments = 32);

    void DrawAABBOutline(SpriteBatch2D& batch,
        const DirectX::XMFLOAT2& center,
        float halfWidth,
        float halfHeight,
        const Color4& color,
        float thickness = 1.0f,
        int layer = 0);

} // namespace KibakoEngine::DebugDraw2D

