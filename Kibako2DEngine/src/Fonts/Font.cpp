// Font atlas utilities
#include "KibakoEngine/Fonts/Font.h"

#include "KibakoEngine/Core/Debug.h"

#include <algorithm>

namespace KibakoEngine {

    bool FontAtlas::Create(ID3D11Device* device,
        int width,
        int height,
        const std::vector<std::uint8_t>& rgbaPixels)
    {
        KBK_ASSERT(device != nullptr, "FontAtlas::Create requires a valid device");
        KBK_ASSERT(width > 0 && height > 0, "FontAtlas::Create requires positive size");
        KBK_ASSERT(!rgbaPixels.empty(), "FontAtlas::Create requires pixel data");
        KBK_ASSERT(static_cast<size_t>(width * height * 4) == rgbaPixels.size(),
            "FontAtlas pixel buffer size mismatch");

        Reset();

        if (!m_texture.CreateFromRGBA8(device, width, height, rgbaPixels.data()))
            return false;

        m_width = width;
        m_height = height;
        return true;
    }

    void FontAtlas::Reset()
    {
        m_texture.Reset();
        m_width = 0;
        m_height = 0;
    }

    const Glyph* Font::GetGlyph(char32_t codepoint) const
    {
        auto it = m_glyphs.find(codepoint);
        if (it == m_glyphs.end())
            return nullptr;
        return &it->second;
    }

    void Font::SetMetrics(float lineHeight, float ascent, float descent)
    {
        m_lineHeight = lineHeight;
        m_ascent = ascent;
        m_descent = descent;
    }

    void Font::AddGlyph(char32_t codepoint, const Glyph& glyph)
    {
        m_glyphs[codepoint] = glyph;
    }

    void Font::SetAtlas(FontAtlas&& atlas)
    {
        m_atlas = std::move(atlas);
    }

} // namespace KibakoEngine

