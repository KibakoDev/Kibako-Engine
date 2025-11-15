// TextRenderer.cpp - Implements batched text rendering helpers.
#include "KibakoEngine/Fonts/Font.h"

#include "KibakoEngine/Core/Debug.h"
#include "KibakoEngine/Core/Profiler.h"
#include "KibakoEngine/Renderer/SpriteBatch2D.h"

#include <string_view>

namespace KibakoEngine {

    void TextRenderer::DrawText(SpriteBatch2D& batch,
        const Font& font,
        std::string_view text,
        const DirectX::XMFLOAT2& position,
        const Color4& color,
        float scale)
    {
        KBK_PROFILE_SCOPE("DrawText");

        if (!font.Atlas().IsValid() || text.empty())
            return;

        const Texture2D& atlas = font.Atlas().Texture();
        const float lineHeight = font.LineHeight() * scale;
        const float ascent = font.Ascent() * scale;
        const float startX = position.x;
        float penX = startX;
        float penY = position.y + ascent;

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
                const float gx = penX + glyph->bearing.x * scale;
                const float gy = penY - glyph->bearing.y * scale;
                const float gw = glyph->size.x * scale;
                const float gh = glyph->size.y * scale;

                const RectF dst = RectF::FromXYWH(gx, gy, gw, gh);
                batch.Push(atlas, dst, glyph->uv, color);
            }

            penX += glyph->advance * scale;
        }
    }

} // namespace KibakoEngine

