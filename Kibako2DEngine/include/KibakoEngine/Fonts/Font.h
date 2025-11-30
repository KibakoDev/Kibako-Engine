// Glyph info and font atlas types
#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#include <DirectXMath.h>

#include "KibakoEngine/Renderer/SpriteTypes.h"
#include "KibakoEngine/Renderer/Texture2D.h"

struct ID3D11Device;

namespace KibakoEngine {

    class SpriteBatch2D;

    struct Glyph
    {
        RectF               uv{};            // UVs in atlas
        DirectX::XMFLOAT2   size{ 0.0f, 0.0f };     // Size in pixels
        DirectX::XMFLOAT2   bearing{ 0.0f, 0.0f };  // Offset from pen
        float               advance = 0.0f;         // Advance in pixels
    };

    class FontAtlas
    {
    public:
        FontAtlas() = default;
        FontAtlas(FontAtlas&&) noexcept = default;
        FontAtlas& operator=(FontAtlas&&) noexcept = default;
        FontAtlas(const FontAtlas&) = delete;
        FontAtlas& operator=(const FontAtlas&) = delete;

        [[nodiscard]] bool Create(ID3D11Device* device,
                                  int width,
                                  int height,
                                  const std::vector<std::uint8_t>& rgbaPixels);
        void Reset();

        [[nodiscard]] const Texture2D& Texture() const { return m_texture; }
        [[nodiscard]] Texture2D& Texture() { return m_texture; }
        [[nodiscard]] int Width() const { return m_width; }
        [[nodiscard]] int Height() const { return m_height; }
        [[nodiscard]] bool IsValid() const { return m_texture.IsValid(); }

    private:
        Texture2D m_texture;
        int       m_width = 0;
        int       m_height = 0;
    };

    class Font
    {
    public:
        Font() = default;
        Font(Font&&) noexcept = default;
        Font& operator=(Font&&) noexcept = default;
        Font(const Font&) = delete;
        Font& operator=(const Font&) = delete;

        [[nodiscard]] const Glyph* GetGlyph(char32_t codepoint) const;
        [[nodiscard]] float LineHeight() const { return m_lineHeight; }
        [[nodiscard]] float Ascent() const { return m_ascent; }
        [[nodiscard]] float Descent() const { return m_descent; }

        [[nodiscard]] const FontAtlas& Atlas() const { return m_atlas; }
        [[nodiscard]] FontAtlas& Atlas() { return m_atlas; }

    private:
        friend class FontLibrary;

        void SetMetrics(float lineHeight, float ascent, float descent);
        void AddGlyph(char32_t codepoint, const Glyph& glyph);
        void SetAtlas(FontAtlas&& atlas);

        FontAtlas m_atlas;
        std::unordered_map<char32_t, Glyph> m_glyphs;
        float m_lineHeight = 0.0f;
        float m_ascent = 0.0f;
        float m_descent = 0.0f;
    };

    class FontLibrary
    {
    public:
        FontLibrary();
        ~FontLibrary();

        bool Init();
        void Shutdown();

        [[nodiscard]] bool IsValid() const;

        [[nodiscard]] std::unique_ptr<Font> LoadFontFromFile(ID3D11Device* device,
                                                             const std::string& path,
                                                             int pixelHeight) const;

    private:
        struct Impl;
        std::unique_ptr<Impl> m_impl;
    };

    class TextRenderer
    {
    public:
        struct TextMetrics
        {
            DirectX::XMFLOAT2 size{ 0.0f, 0.0f };
            float lineHeight = 0.0f;
            float ascent = 0.0f;
            float descent = 0.0f;
            std::size_t lineCount = 0;
        };

        struct TextRenderSettings
        {
            Color4 color = Color4::White();
            float scale = 1.0f;
            bool snapToPixel = true;
            int layer = 0;
        };

        [[nodiscard]] static TextMetrics MeasureText(const Font& font,
                                                     std::string_view text,
                                                     float scale = 1.0f);

        static void DrawText(SpriteBatch2D& batch,
                             const Font& font,
                             std::string_view text,
                             const DirectX::XMFLOAT2& position,
                             const Color4& color,
                             float scale = 1.0f,
                             int layer = 0);

        static void DrawText(SpriteBatch2D& batch,
                             const Font& font,
                             std::string_view text,
                             const DirectX::XMFLOAT2& position,
                             const TextRenderSettings& settings);
    };

} // namespace KibakoEngine

