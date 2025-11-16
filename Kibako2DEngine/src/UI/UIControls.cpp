// UIControls.cpp - Implements basic UI widgets for the HUD system.
#include "KibakoEngine/UI/UIControls.h"

#include <SDL2/SDL_mouse.h>

#include "KibakoEngine/Renderer/SpriteBatch2D.h"

namespace KibakoEngine {

    void UILabel::Render(SpriteBatch2D& batch, const UIContext& ctx) const
    {
        if (!m_visible || !m_font || m_text.empty())
            return;

        const DirectX::XMFLOAT2 pos = WorldPosition(ctx);
        TextRenderer::DrawText(batch, *m_font, m_text, pos, m_color, m_scale);

        UIElement::Render(batch, ctx);
    }

    void UIImage::Render(SpriteBatch2D& batch, const UIContext& ctx) const
    {
        if (!m_visible || !m_texture || !m_texture->IsValid())
            return;

        const RectF dst = WorldRect(ctx);
        const RectF src = RectF::FromXYWH(0.0f, 0.0f, 1.0f, 1.0f);
        batch.Push(*m_texture, dst, src, m_color, 0.0f, m_layer);

        UIElement::Render(batch, ctx);
    }

    void UIPanel::Render(SpriteBatch2D& batch, const UIContext& ctx) const
    {
        if (!m_visible)
            return;

        const RectF dst = WorldRect(ctx);
        const RectF src = RectF::FromXYWH(0.0f, 0.0f, 1.0f, 1.0f);
        const Texture2D* white = batch.DefaultWhiteTexture();
        if (white)
            batch.Push(*white, dst, src, m_color, 0.0f, m_layer);

        UIElement::Render(batch, ctx);
    }

    UIButton::UIButton(std::string name)
        : UIElement(std::move(name))
    {
        m_size = DirectX::XMFLOAT2{ 160.0f, 42.0f };
    }

    bool UIButton::HitTest(const UIContext& ctx) const
    {
        if (!ctx.input)
            return false;

        const RectF rect = WorldRect(ctx);
        const float mx = static_cast<float>(ctx.input->MouseX());
        const float my = static_cast<float>(ctx.input->MouseY());

        return mx >= rect.x && mx <= (rect.x + rect.w) &&
            my >= rect.y && my <= (rect.y + rect.h);
    }

    Color4 UIButton::CurrentColor() const
    {
        if (m_pressed)
            return m_colorPressed;
        if (m_hovered)
            return m_colorHover;
        return m_colorNormal;
    }

    DirectX::XMFLOAT2 UIButton::MeasureText() const
    {
        if (!m_font)
            return DirectX::XMFLOAT2{ 0.0f, 0.0f };

        float width = 0.0f;
        for (char c : m_text) {
            const Glyph* glyph = m_font->GetGlyph(static_cast<char32_t>(c));
            if (!glyph)
                continue;
            width += glyph->advance;
        }

        const float height = m_font->LineHeight();
        return DirectX::XMFLOAT2{ width * m_textScale, height * m_textScale };
    }

    DirectX::XMFLOAT2 UIButton::TextPosition(const UIContext& ctx) const
    {
        const DirectX::XMFLOAT2 pos = WorldPosition(ctx);

        if (!m_font)
            return pos;

        const DirectX::XMFLOAT2 textSize = MeasureText();

        if (!m_centerText)
            return DirectX::XMFLOAT2{ pos.x + m_padding.x, pos.y + m_padding.y };

        return DirectX::XMFLOAT2{
            pos.x + 0.5f * (m_size.x - textSize.x),
            pos.y + 0.5f * (m_size.y - textSize.y)
        };
    }

    void UIButton::Update(const UIContext& ctx)
    {
        if (!m_visible)
            return;

        const bool inside = HitTest(ctx);

        m_hovered = inside;
        m_pressed = false;

        if (ctx.input) {
            if (inside && ctx.input->MousePressed(SDL_BUTTON_LEFT))
                m_trackingPress = true;

            if (m_trackingPress) {
                if (ctx.input->MouseDown(SDL_BUTTON_LEFT)) {
                    m_pressed = true;
                }
                else {
                    if (inside && m_onClick)
                        m_onClick();

                    m_pressed = false;
                    m_trackingPress = false;
                }
            }
            else {
                m_pressed = false;
            }
        }

        UIElement::Update(ctx);
    }

    void UIButton::Render(SpriteBatch2D& batch, const UIContext& ctx) const
    {
        if (!m_visible)
            return;

        const RectF dst = WorldRect(ctx);
        const RectF src = RectF::FromXYWH(0.0f, 0.0f, 1.0f, 1.0f);
        const Texture2D* white = batch.DefaultWhiteTexture();
        if (white)
            batch.Push(*white, dst, src, CurrentColor(), 0.0f, m_layer);

        if (m_font && !m_text.empty()) {
            const DirectX::XMFLOAT2 textPos = TextPosition(ctx);
            TextRenderer::DrawText(batch, *m_font, m_text, textPos, m_textColor, m_textScale);
        }

        UIElement::Render(batch, ctx);
    }

} // namespace KibakoEngine

