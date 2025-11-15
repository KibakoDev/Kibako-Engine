// FontLibrary.cpp - Implements FreeType font loading into GPU atlases.
#include "KibakoEngine/Fonts/Font.h"

#include "KibakoEngine/Core/Debug.h"
#include "KibakoEngine/Core/Log.h"
#include "KibakoEngine/Core/Profiler.h"

#include <algorithm>
#include <cstdint>
#include <cstring>
#include <utility>
#include <vector>

#include <ft2build.h>
#include FT_FREETYPE_H

namespace KibakoEngine {

    namespace
    {
        constexpr const char* kLogChannel = "Font";
        constexpr char32_t kGlyphStart = 32;
        constexpr char32_t kGlyphEnd = 126;
        constexpr int kAtlasPadding = 1;

        struct GlyphBitmap
        {
            char32_t code = 0;
            int width = 0;
            int height = 0;
            int bearingX = 0;
            int bearingY = 0;
            int advance = 0;
            std::vector<std::uint8_t> buffer;
        };
    }

    struct FontLibrary::Impl
    {
        FT_Library library = nullptr;
    };

    FontLibrary::FontLibrary() = default;

    FontLibrary::~FontLibrary()
    {
        Shutdown();
    }

    bool FontLibrary::Init()
    {
        if (IsValid())
            return true;

        auto impl = std::make_unique<Impl>();
        FT_Error err = FT_Init_FreeType(&impl->library);
        if (err != 0) {
            KbkError(kLogChannel, "FT_Init_FreeType failed: %d", static_cast<int>(err));
            return false;
        }

        m_impl = std::move(impl);
        KbkLog(kLogChannel, "FreeType initialized");
        return true;
    }

    void FontLibrary::Shutdown()
    {
        if (m_impl && m_impl->library) {
            FT_Done_FreeType(m_impl->library);
            m_impl->library = nullptr;
            KbkLog(kLogChannel, "FreeType shutdown");
        }
        m_impl.reset();
    }

    bool FontLibrary::IsValid() const
    {
        return m_impl != nullptr && m_impl->library != nullptr;
    }

    std::unique_ptr<Font> FontLibrary::LoadFontFromFile(ID3D11Device* device,
        const std::string& path,
        int pixelHeight) const
    {
        KBK_PROFILE_SCOPE("FontLoadTTF");

        KBK_ASSERT(device != nullptr, "FontLibrary::LoadFontFromFile requires device");
        KBK_ASSERT(pixelHeight > 0, "FontLibrary::LoadFontFromFile requires positive size");

        if (!IsValid()) {
            KbkError(kLogChannel, "Cannot load font '%s': library not initialized", path.c_str());
            return nullptr;
        }

        FT_Face face = nullptr;
        FT_Error err = FT_New_Face(m_impl->library, path.c_str(), 0, &face);
        if (err != 0) {
            KbkError(kLogChannel, "FT_New_Face failed for '%s': %d", path.c_str(), static_cast<int>(err));
            return nullptr;
        }

        err = FT_Set_Pixel_Sizes(face, 0, static_cast<FT_UInt>(pixelHeight));
        if (err != 0) {
            KbkError(kLogChannel, "FT_Set_Pixel_Sizes failed for '%s': %d", path.c_str(), static_cast<int>(err));
            FT_Done_Face(face);
            return nullptr;
        }

        std::vector<GlyphBitmap> glyphs;
        glyphs.reserve(static_cast<size_t>(kGlyphEnd - kGlyphStart + 1));

        int atlasWidth = kAtlasPadding;
        int atlasHeight = 0;

        for (char32_t code = kGlyphStart; code <= kGlyphEnd; ++code) {
            err = FT_Load_Char(face, static_cast<FT_ULong>(code), FT_LOAD_RENDER);
            if (err != 0) {
                KbkWarn(kLogChannel, "FT_Load_Char failed for code %u", static_cast<unsigned>(code));
                continue;
            }

            FT_GlyphSlot slot = face->glyph;
            const FT_Bitmap& bitmap = slot->bitmap;

            GlyphBitmap entry;
            entry.code = code;
            entry.width = static_cast<int>(bitmap.width);
            entry.height = static_cast<int>(bitmap.rows);
            entry.bearingX = slot->bitmap_left;
            entry.bearingY = slot->bitmap_top;
            entry.advance = static_cast<int>(slot->advance.x >> 6);

            if (entry.width > 0 && entry.height > 0) {
                entry.buffer.resize(static_cast<size_t>(entry.width * entry.height));
                for (int y = 0; y < entry.height; ++y) {
                    const int pitch = bitmap.pitch;
                    const std::uint8_t* src = pitch >= 0
                        ? bitmap.buffer + y * pitch
                        : bitmap.buffer + (entry.height - 1 - y) * (-pitch);
                    std::uint8_t* dst = entry.buffer.data() + y * entry.width;
                    std::memcpy(dst, src, static_cast<size_t>(entry.width));
                }

                atlasWidth += entry.width + kAtlasPadding;
                atlasHeight = std::max(atlasHeight, entry.height + 2 * kAtlasPadding);
            }

            glyphs.push_back(std::move(entry));
        }

        if (atlasHeight == 0)
            atlasHeight = pixelHeight + 2 * kAtlasPadding;
        if (atlasWidth <= kAtlasPadding)
            atlasWidth = pixelHeight + 2 * kAtlasPadding;

        std::vector<std::uint8_t> atlasPixels(static_cast<size_t>(atlasWidth * atlasHeight * 4), 0);

        Font font;
        FontAtlas atlas;

        int penX = kAtlasPadding;
        const int penY = kAtlasPadding;

        for (const GlyphBitmap& gb : glyphs) {
            Glyph glyph{};
            glyph.size = { static_cast<float>(gb.width), static_cast<float>(gb.height) };
            glyph.bearing = { static_cast<float>(gb.bearingX), static_cast<float>(gb.bearingY) };
            glyph.advance = static_cast<float>(gb.advance);

            if (gb.width > 0 && gb.height > 0 && !gb.buffer.empty()) {
                for (int y = 0; y < gb.height; ++y) {
                    const std::uint8_t* src = gb.buffer.data() + y * gb.width;
                    std::uint8_t* row = atlasPixels.data() +
                        static_cast<size_t>(((penY + y) * atlasWidth + penX) * 4);
                    for (int x = 0; x < gb.width; ++x) {
                        const std::uint8_t value = src[x];
                        std::uint8_t* dst = row + static_cast<size_t>(x * 4);
                        dst[0] = 255;
                        dst[1] = 255;
                        dst[2] = 255;
                        dst[3] = value;
                    }
                }

                const float u = static_cast<float>(penX) / static_cast<float>(atlasWidth);
                const float v = static_cast<float>(penY) / static_cast<float>(atlasHeight);
                const float w = static_cast<float>(gb.width) / static_cast<float>(atlasWidth);
                const float h = static_cast<float>(gb.height) / static_cast<float>(atlasHeight);
                glyph.uv = RectF::FromXYWH(u, v, w, h);

                penX += gb.width + kAtlasPadding;
            }
            else {
                glyph.uv = RectF::FromXYWH(0.0f, 0.0f, 0.0f, 0.0f);
            }

            font.AddGlyph(gb.code, glyph);
        }

        if (!atlas.Create(device, atlasWidth, atlasHeight, atlasPixels)) {
            KbkError(kLogChannel, "Failed to create atlas for '%s'", path.c_str());
            FT_Done_Face(face);
            return nullptr;
        }

        const float rawLineHeight = static_cast<float>(face->size->metrics.height >> 6);
        const float rawAscent = static_cast<float>(face->size->metrics.ascender >> 6);
        const float rawDescent = static_cast<float>(face->size->metrics.descender >> 6);

        const float lineHeight = rawLineHeight > 0.0f ? rawLineHeight : static_cast<float>(pixelHeight);
        const float ascent = rawAscent > 0.0f ? rawAscent : lineHeight;
        const float descent = rawDescent < 0.0f ? rawDescent : (rawDescent > 0.0f ? -rawDescent : -lineHeight * 0.25f);

        font.SetAtlas(std::move(atlas));
        font.SetMetrics(lineHeight, ascent, descent);

        FT_Done_Face(face);

        auto result = std::make_unique<Font>(std::move(font));
        KbkLog(kLogChannel,
            "Loaded font '%s' (%d px) -> %dx%d atlas",
            path.c_str(),
            pixelHeight,
            result->Atlas().Width(),
            result->Atlas().Height());
        return result;
    }

} // namespace KibakoEngine

