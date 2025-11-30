// Basic UI widgets
#include "KibakoEngine/UI/UIControls.h"

#include <algorithm>
#include <cmath>

#include <SDL2/SDL_mouse.h>

#include "KibakoEngine/Renderer/SpriteBatch2D.h"

namespace KibakoEngine {

    namespace
    {
        inline float SnapToPixel(float value)
        {
            return std::roundf(value);
        }

        RectF SnapRect(const RectF& rect)
        {
            const float x0 = SnapToPixel(rect.x);
            const float y0 = SnapToPixel(rect.y);
            const float x1 = SnapToPixel(rect.x + rect.w);
            const float y1 = SnapToPixel(rect.y + rect.h);
            return RectF::FromXYWH(x0, y0, x1 - x0, y1 - y0);
        }
    }

    void UILabel::OnUpdate(const UIContext& ctx)
    {
        (void)ctx;

        const Font* font = m_font;
        if (!font)
            return;

        if (m_autoSize && !m_text.empty()) {
            const auto metrics = TextRenderer::MeasureText(*font, m_text, m_scale);
            const DirectX::XMFLOAT2 newSize{ metrics.size.x, metrics.lineHeight };
            if (newSize.x != m_size.x || newSize.y != m_size.y)
                SetSize(newSize);
        }
    }

    void UILabel::OnRender(SpriteBatch2D& batch, const UIContext& ctx, const UIStyle& style) const
    {
        const Font* font = m_font ? m_font : style.font;
        if (!font || m_text.empty())
            return;

        DirectX::XMFLOAT2 pos = WorldPosition(ctx);
        if (m_snapToPixel) {
            pos.x = SnapToPixel(pos.x);
            pos.y = SnapToPixel(pos.y);
        }

        const Color4 finalColor = m_tint.value_or(m_color);

        TextRenderer::DrawText(batch, *font, m_text, pos,
            TextRenderer::TextRenderSettings{ finalColor, m_scale, m_snapToPixel, m_layer });

    }

    void UIImage::OnRender(SpriteBatch2D& batch, const UIContext& ctx, const UIStyle&) const
    {
        if (!m_texture || !m_texture->IsValid())
            return;

        RectF dst = WorldRect(ctx);
        if (m_snapToPixel)
            dst = SnapRect(dst);
        const RectF src = RectF::FromXYWH(0.0f, 0.0f, 1.0f, 1.0f);
        const Color4 color = m_tint.value_or(m_color);
        batch.Push(*m_texture, dst, src, color, 0.0f, m_layer);
    }

    void UIPanel::OnRender(SpriteBatch2D& batch, const UIContext& ctx, const UIStyle&) const
    {
        RectF dst = WorldRect(ctx);
        if (m_snapToPixel)
            dst = SnapRect(dst);
        const RectF src = RectF::FromXYWH(0.0f, 0.0f, 1.0f, 1.0f);
        const Texture2D* white = batch.DefaultWhiteTexture();
        if (white)
            batch.Push(*white, dst, src, m_tint.value_or(m_color), 0.0f, m_layer);
    }

    UIButton::UIButton(std::string name)
        : UIElement(std::move(name))
    {
        m_size = DirectX::XMFLOAT2{ 160.0f, 42.0f };
        m_text = m_name.empty() ? "Button" : m_name;

        m_textLabel = &EmplaceChild<UILabel>(m_name + ".Text");
        m_textLabel->SetAnchor(UIAnchor::Center);
        m_textLabel->SetPixelSnap(true);
        m_textLabel->SetText(m_text);
        m_textLabel->SetScale(m_textScale);
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

        if (m_textDirty) {
            const auto metrics = TextRenderer::MeasureText(*m_font, m_text, m_textScale);
            m_cachedTextSize = metrics.size;
            m_textDirty = false;
        }

        return m_cachedTextSize;
    }

    void UIButton::OnUpdate(const UIContext& ctx)
    {
        if (m_labelDirty || m_textDirty)
            RefreshTextLabel();

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

    }

    void UIButton::OnRender(SpriteBatch2D& batch, const UIContext& ctx, const UIStyle& style) const
    {
        (void)style;

        const RectF dst = WorldRect(ctx);
        const RectF snappedDst = m_snapToPixel ? SnapRect(dst) : dst;
        const RectF src = RectF::FromXYWH(0.0f, 0.0f, 1.0f, 1.0f);
        const Texture2D* white = batch.DefaultWhiteTexture();
        if (white)
            batch.Push(*white, snappedDst, src, m_tint.value_or(CurrentColor()), 0.0f, m_layer);
    }

    void UIButton::SetStyle(const UIStyle& style)
    {
        if (style.font)
            SetFont(style.font);

        SetTextScale(style.buttonTextScale);
        SetPadding(style.buttonPadding);
        SetSize(style.buttonSize);
        SetTextColor(style.primaryTextColor);
        SetNormalColor(style.buttonNormal);
        SetHoverColor(style.buttonHover);
        SetPressedColor(style.buttonPressed);
    }

    void UIButton::RefreshTextLabel()
    {
        if (!m_textLabel)
            return;

        m_textLabel->SetText(m_text);
        m_textLabel->SetColor(m_tint.value_or(m_textColor));
        m_textLabel->SetScale(FittedTextScale());
        m_textLabel->SetPixelSnap(m_snapToPixel);

        if (m_font)
            m_textLabel->SetFont(m_font);

        if (m_centerText) {
            m_textLabel->SetAnchor(UIAnchor::Center);
            m_textLabel->SetPosition(m_labelOffset);
        }
        else {
            m_textLabel->SetAnchor(UIAnchor::TopLeft);
            m_textLabel->SetPosition(m_padding);
        }

        m_labelDirty = false;
    }

    float UIButton::FittedTextScale() const
    {
        float scale = m_textScale;

        if (m_autoFitText && m_font && !m_text.empty()) {
            const DirectX::XMFLOAT2 textSize = MeasureText();
            const float availableWidth = std::max(0.0f, m_size.x - (m_padding.x * 2.0f));

            if (textSize.x > 0.0f && textSize.x > availableWidth) {
                const float fitted = m_textScale * (availableWidth / textSize.x);
                scale = std::max(m_minTextScale, fitted);
            }
        }

        return scale;
    }

    void UIStyle::ApplyHeading(UILabel& label) const
    {
        label.SetFont(font);
        label.SetColor(headingColor);
        label.SetScale(headingScale);
    }

    void UIStyle::ApplyBody(UILabel& label) const
    {
        label.SetFont(font);
        label.SetColor(primaryTextColor);
        label.SetScale(bodyScale);
    }

    void UIStyle::ApplyCaption(UILabel& label) const
    {
        label.SetFont(font);
        label.SetColor(mutedTextColor);
        label.SetScale(captionScale);
    }

    void UIStyle::ApplyPanel(UIPanel& panel) const
    {
        panel.SetColor(panelColor);
        panel.SetLayer(9500);
    }

    void UIStyle::ApplyButton(UIButton& button) const
    {
        if (font)
            button.SetFont(font);

        button.SetSize(buttonSize);
        button.SetPadding(buttonPadding);
        button.SetTextScale(buttonTextScale);
        button.SetTextColor(primaryTextColor);
        button.SetNormalColor(buttonNormal);
        button.SetHoverColor(buttonHover);
        button.SetPressedColor(buttonPressed);
    }

} // namespace KibakoEngine

