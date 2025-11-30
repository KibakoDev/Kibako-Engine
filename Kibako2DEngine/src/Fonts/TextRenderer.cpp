// Batched text rendering
#include "KibakoEngine/Fonts/Font.h"

#include "KibakoEngine/Core/Debug.h"
#include "KibakoEngine/Core/Profiler.h"
#include "KibakoEngine/Renderer/SpriteBatch2D.h"

#include <algorithm>
#include <cmath>
#include <string_view>

namespace KibakoEngine {

    namespace
    {
        inline float SnapToPixel(float value)
        {
            return std::roundf(value);
        }

        TextRenderer::TextMetrics MeasureInternal(const Font& font, std::string_view text, float scale)
        {
            TextRenderer::TextMetrics metrics{};

            if (!font.Atlas().IsValid() || text.empty())
                return metrics;

            const float lineHeight = font.LineHeight() * scale;
            metrics.lineHeight = lineHeight;
            metrics.ascent = font.Ascent() * scale;
            metrics.descent = font.Descent() * scale;

            float lineWidth = 0.0f;
            metrics.lineCount = 1;

            for (char ch : text) {
                if (ch == '\n') {
                    metrics.size.x = std::max(metrics.size.x, lineWidth);
                    lineWidth = 0.0f;
                    ++metrics.lineCount;
                    continue;
                }

                const Glyph* glyph = font.GetGlyph(static_cast<unsigned char>(ch));
                if (!glyph) {
                    lineWidth += lineHeight * 0.5f;
                    continue;
                }

                lineWidth += glyph->advance * scale;
            }

            metrics.size.x = std::max(metrics.size.x, lineWidth);
            metrics.size.y = metrics.lineHeight * static_cast<float>(metrics.lineCount);

            return metrics;
        }
    }

    TextRenderer::TextMetrics TextRenderer::MeasureText(const Font& font,
        std::string_view text,
        float scale)
    {
        return MeasureInternal(font, text, scale);
    }

    void TextRenderer::DrawText(SpriteBatch2D& batch,
        const Font& font,
        std::string_view text,
        const DirectX::XMFLOAT2& position,
        const Color4& color,
        float scale,
        int layer)
    {
        DrawText(batch, font, text, position, TextRenderSettings{ color, scale, true, layer });
    }

    void TextRenderer::DrawText(SpriteBatch2D& batch,
        const Font& font,
        std::string_view text,
        const DirectX::XMFLOAT2& position,
        const TextRenderSettings& settings)
    {
        KBK_PROFILE_SCOPE("DrawText");

        if (!font.Atlas().IsValid() || text.empty())
            return;

        const Texture2D& atlas = font.Atlas().Texture();
        const float lineHeight = font.LineHeight() * settings.scale;
        const float ascent = font.Ascent() * settings.scale;
        const float startX = settings.snapToPixel ? SnapToPixel(position.x) : position.x;
        float penX = startX;
        float penY = settings.snapToPixel ? SnapToPixel(position.y + ascent) : (position.y + ascent);

        for (char ch : text) {
            if (ch == '\n') {
                penX = startX;
                penY += lineHeight;
                continue;
            }

            const Glyph* glyph = font.GetGlyph(static_cast<unsigned char>(ch));
            if (!glyph) {
                penX += lineHeight * 0.5f;
                continue;
            }

            if (glyph->size.x > 0.0f && glyph->size.y > 0.0f) {
                float gx = penX + glyph->bearing.x * settings.scale;
                float gy = penY - glyph->bearing.y * settings.scale;
                float gw = glyph->size.x * settings.scale;
                float gh = glyph->size.y * settings.scale;

                if (settings.snapToPixel) {
                    const float snappedX0 = SnapToPixel(gx);
                    const float snappedY0 = SnapToPixel(gy);
                    const float snappedX1 = SnapToPixel(gx + gw);
                    const float snappedY1 = SnapToPixel(gy + gh);
                    gx = snappedX0;
                    gy = snappedY0;
                    gw = snappedX1 - snappedX0;
                    gh = snappedY1 - snappedY0;
                }

                const RectF dst = RectF::FromXYWH(gx, gy, gw, gh);
                batch.Push(atlas, dst, glyph->uv, settings.color, 0.0f, settings.layer);
            }

            penX += glyph->advance * settings.scale;
        }
    }

} // namespace KibakoEngine

