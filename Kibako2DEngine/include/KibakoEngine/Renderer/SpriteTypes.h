// Kibako2DEngine/include/KibakoEngine/Renderer/SpriteTypes.h
#pragma once
#include <algorithm>

namespace KibakoEngine {

    // =====================================================
    // Basic rectangle (float-based, used for sprite dst/src)
    // =====================================================
    struct RectF {
        float x = 0.0f;
        float y = 0.0f;
        float w = 0.0f;
        float h = 0.0f;

        RectF() = default;
        RectF(float X, float Y, float W, float H)
            : x(X), y(Y), w(W), h(H) {
        }

        float Left()   const { return x; }
        float Right()  const { return x + w; }
        float Top()    const { return y; }
        float Bottom() const { return y + h; }

        bool Contains(float px, float py) const {
            return (px >= x && px <= x + w &&
                py >= y && py <= y + h);
        }

        bool Intersects(const RectF& other) const {
            return !(Right() < other.Left() ||
                Left() > other.Right() ||
                Bottom() < other.Top() ||
                Top() > other.Bottom());
        }
    };

    // =====================================================
    // Basic RGBA color (float 0..1)
    // =====================================================
    struct Color4 {
        float r = 1.0f;
        float g = 1.0f;
        float b = 1.0f;
        float a = 1.0f;

        Color4() = default;
        Color4(float R, float G, float B, float A = 1.0f)
            : r(R), g(G), b(B), a(A) {
        }

        static Color4 White() { return { 1.0f, 1.0f, 1.0f, 1.0f }; }
        static Color4 Black() { return { 0.0f, 0.0f, 0.0f, 1.0f }; }
        static Color4 Red() { return { 1.0f, 0.0f, 0.0f, 1.0f }; }
        static Color4 Green() { return { 0.0f, 1.0f, 0.0f, 1.0f }; }
        static Color4 Blue() { return { 0.0f, 0.0f, 1.0f, 1.0f }; }
        static Color4 Gray(float v) { return { v, v, v, 1.0f }; }

        // Simple linear interpolation
        static Color4 Lerp(const Color4& a, const Color4& b, float t) {
            return {
                a.r + (b.r - a.r) * t,
                a.g + (b.g - a.g) * t,
                a.b + (b.b - a.b) * t,
                a.a + (b.a - a.a) * t
            };
        }

        bool operator==(const Color4& o) const {
            return r == o.r && g == o.g && b == o.b && a == o.a;
        }

        bool operator!=(const Color4& o) const {
            return !(*this == o);
        }
    };

}