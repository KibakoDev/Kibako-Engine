#include "KibakoEngine/Collision/Collision2D.h"
#include "KibakoEngine/Scene/Scene2D.h" // Pour la vraie définition de Transform2D

#if KBK_DEBUG_BUILD
#include "KibakoEngine/Core/Input.h"
#include "KibakoEngine/Renderer/SpriteBatch2D.h"
#include "KibakoEngine/Renderer/Texture2D.h"

#include <SDL2/SDL_scancode.h>

#include <cmath>
#include <limits>
#include <vector>
#endif

namespace KibakoEngine {

#if KBK_DEBUG_BUILD
namespace CollisionDebug2D
{
    namespace
    {
        struct CircleTest
        {
            DirectX::XMFLOAT2 centerA{};
            DirectX::XMFLOAT2 centerB{};
            float radiusA = 0.0f;
            float radiusB = 0.0f;
            bool  hit = false;
        };

        struct AABBTest
        {
            DirectX::XMFLOAT2 centerA{};
            DirectX::XMFLOAT2 centerB{};
            DirectX::XMFLOAT2 halfExtentsA{};
            DirectX::XMFLOAT2 halfExtentsB{};
            bool  hit = false;
        };

        struct DebugState
        {
            bool enabled = false;
            bool textureReady = false;
            Texture2D pixel;
            std::vector<CircleTest> circleTests;
            std::vector<AABBTest>   aabbTests;
        };

        DebugState g_state;

        constexpr int kDebugLayer = std::numeric_limits<int>::max() / 2;
        constexpr float kTwoPi = 6.28318530717958647692f;

        Texture2D* EnsureTexture(SpriteBatch2D& batch)
        {
            if (!g_state.textureReady) {
                if (ID3D11Device* device = batch.GetDevice()) {
                    if (g_state.pixel.CreateSolidColor(device, 1, 1, 0xFFFFFFFFu)) {
                        g_state.textureReady = true;
                    }
                }
            }

            return g_state.textureReady && g_state.pixel.IsValid() ? &g_state.pixel : nullptr;
        }

        void PushLine(SpriteBatch2D& batch,
                      Texture2D& texture,
                      const DirectX::XMFLOAT2& start,
                      const DirectX::XMFLOAT2& end,
                      const Color4& color,
                      float thickness)
        {
            const float dx = end.x - start.x;
            const float dy = end.y - start.y;
            const float lengthSq = dx * dx + dy * dy;
            if (lengthSq <= 0.0001f)
                return;

            const float length = std::sqrt(lengthSq);
            const float angle = std::atan2(dy, dx);
            const float cx = (start.x + end.x) * 0.5f;
            const float cy = (start.y + end.y) * 0.5f;

            RectF dst{};
            dst.w = length;
            dst.h = thickness;
            dst.x = cx - dst.w * 0.5f;
            dst.y = cy - dst.h * 0.5f;

            static const RectF kSrc = RectF::FromXYWH(0.0f, 0.0f, 1.0f, 1.0f);
            batch.Push(texture, dst, kSrc, color, angle, kDebugLayer);
        }

        void DrawCircle(SpriteBatch2D& batch,
                        Texture2D& texture,
                        const DirectX::XMFLOAT2& center,
                        float radius,
                        const Color4& color)
        {
            if (radius <= 0.0f)
                return;

            constexpr int segments = 24;
            const float step = kTwoPi / static_cast<float>(segments);
            DirectX::XMFLOAT2 prev{ center.x + radius, center.y };
            for (int i = 1; i <= segments; ++i) {
                const float angle = step * static_cast<float>(i);
                DirectX::XMFLOAT2 current{
                    center.x + std::cos(angle) * radius,
                    center.y + std::sin(angle) * radius };
                PushLine(batch, texture, prev, current, color, 1.5f);
                prev = current;
            }
        }

        void DrawAABB(SpriteBatch2D& batch,
                      Texture2D& texture,
                      const DirectX::XMFLOAT2& center,
                      const DirectX::XMFLOAT2& halfExtents,
                      const Color4& color)
        {
            if (halfExtents.x <= 0.0f || halfExtents.y <= 0.0f)
                return;

            const DirectX::XMFLOAT2 tl{ center.x - halfExtents.x, center.y - halfExtents.y };
            const DirectX::XMFLOAT2 tr{ center.x + halfExtents.x, center.y - halfExtents.y };
            const DirectX::XMFLOAT2 br{ center.x + halfExtents.x, center.y + halfExtents.y };
            const DirectX::XMFLOAT2 bl{ center.x - halfExtents.x, center.y + halfExtents.y };

            PushLine(batch, texture, tl, tr, color, 1.5f);
            PushLine(batch, texture, tr, br, color, 1.5f);
            PushLine(batch, texture, br, bl, color, 1.5f);
            PushLine(batch, texture, bl, tl, color, 1.5f);
        }
    } // namespace

    void UpdateToggle(const Input& input)
    {
        if (input.KeyPressed(SDL_SCANCODE_F1)) {
            g_state.enabled = !g_state.enabled;
        }
    }

    void BeginFrame()
    {
        g_state.circleTests.clear();
        g_state.aabbTests.clear();
    }

    void RenderDebug(SpriteBatch2D& batch)
    {
        if (!g_state.enabled)
            return;

        Texture2D* texture = EnsureTexture(batch);
        if (!texture)
            return;

        for (const CircleTest& test : g_state.circleTests) {
            const Color4 color = test.hit ? Color4{ 1.0f, 0.2f, 0.2f, 1.0f } : Color4{ 0.2f, 1.0f, 0.3f, 1.0f };
            PushLine(batch, *texture, test.centerA, test.centerB, color, 2.0f);
            DrawCircle(batch, *texture, test.centerA, test.radiusA, color);
            DrawCircle(batch, *texture, test.centerB, test.radiusB, color);
        }

        for (const AABBTest& test : g_state.aabbTests) {
            const Color4 color = test.hit ? Color4{ 1.0f, 0.4f, 0.2f, 1.0f } : Color4{ 0.2f, 0.6f, 1.0f, 1.0f };
            DrawAABB(batch, *texture, test.centerA, test.halfExtentsA, color);
            DrawAABB(batch, *texture, test.centerB, test.halfExtentsB, color);
        }
    }

    void RecordCircleTest(const CircleCollider2D& c1, const Transform2D& t1,
        const CircleCollider2D& c2, const Transform2D& t2, bool hit)
    {
        if (!g_state.enabled)
            return;

        g_state.circleTests.push_back({ t1.position, t2.position, c1.radius, c2.radius, hit });
    }

    void RecordAABBTest(const AABBCollider2D& b1, const Transform2D& t1,
        const AABBCollider2D& b2, const Transform2D& t2, bool hit)
    {
        if (!g_state.enabled)
            return;

        g_state.aabbTests.push_back({ t1.position, t2.position, { b1.halfW, b1.halfH }, { b2.halfW, b2.halfH }, hit });
    }
} // namespace CollisionDebug2D
#endif

    bool Intersects(const CircleCollider2D& c1, const Transform2D& t1,
        const CircleCollider2D& c2, const Transform2D& t2)
    {
        if (!c1.active || !c2.active)
            return false;

        float dx = t1.position.x - t2.position.x;
        float dy = t1.position.y - t2.position.y;
        float dist2 = dx * dx + dy * dy;

        float r = c1.radius + c2.radius;
        const bool hit = dist2 <= (r * r);

#if KBK_DEBUG_BUILD
        CollisionDebug2D::RecordCircleTest(c1, t1, c2, t2, hit);
#endif
        return hit;
    }

    bool Intersects(const AABBCollider2D& b1, const Transform2D& t1,
        const AABBCollider2D& b2, const Transform2D& t2)
    {
        if (!b1.active || !b2.active)
            return false;

        // Positions centres
        float ax1 = t1.position.x - b1.halfW;
        float ax2 = t1.position.x + b1.halfW;

        float ay1 = t1.position.y - b1.halfH;
        float ay2 = t1.position.y + b1.halfH;

        float bx1 = t2.position.x - b2.halfW;
        float bx2 = t2.position.x + b2.halfW;

        float by1 = t2.position.y - b2.halfH;
        float by2 = t2.position.y + b2.halfH;

        const bool hit = (ax1 <= bx2 && ax2 >= bx1 && ay1 <= by2 && ay2 >= by1);

#if KBK_DEBUG_BUILD
        CollisionDebug2D::RecordAABBTest(b1, t1, b2, t2, hit);
#endif
        return hit;
    }

} // namespace KibakoEngine
