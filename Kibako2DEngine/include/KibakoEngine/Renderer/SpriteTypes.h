// =====================================================
// Kibako2DEngine/Renderer/SpriteTypes.h
// Basic geometric and color helper types.
// =====================================================
#pragma once
#include <algorithm>

namespace KibakoEngine {

    // Simple rectangle (float)
    struct RectF {
        float x = 0.0f, y = 0.0f, w = 0.0f, h = 0.0f;

        RectF() = default;
        RectF(float X, float Y, float W, float H) : x(X), y(Y), w(W), h(H) {}

        float Left()   const { return x; }
        float Right()  const { return x + w; }
        float Top()    const { return y; }
        float Bottom() const { return y + h; }

        bool Contains(float px, float py) const {
            return (px >= x && px <= x + w && py >= y && py <= y + h);
        }

        bool Intersects(const RectF& o) const {
            return !(Right() < o.Left() || Left() > o.Right() ||
                Bottom() < o.Top() || Top() > o.Bottom());
        }
    };

    // Simple RGBA color (float 0..1)
    struct Color4 {
        float r = 1.0f, g = 1.0f, b = 1.0f, a = 1.0f;

        Color4() = default;
        Color4(float R, float G, float B, float A = 1.0f) : r(R), g(G), b(B), a(A) {}

        static Color4 White() { return { 1, 1, 1, 1 }; }
        static Color4 Black() { return { 0, 0, 0, 1 }; }
        static Color4 Red() { return { 1, 0, 0, 1 }; }
        static Color4 Green() { return { 0, 1, 0, 1 }; }
        static Color4 Blue() { return { 0, 0, 1, 1 }; }
        static Color4 Gray(float v) { return { v, v, v, 1 }; }

        static Color4 Lerp(const Color4& a, const Color4& b, float t) {
            return { a.r + (b.r - a.r) * t,
                     a.g + (b.g - a.g) * t,
                     a.b + (b.b - a.b) * t,
                     a.a + (b.a - a.a) * t };
        }

        bool operator==(const Color4& o) const {
            return r == o.r && g == o.g && b == o.b && a == o.a;
        }

        bool operator!=(const Color4& o) const { return !(*this == o); }
    };

}