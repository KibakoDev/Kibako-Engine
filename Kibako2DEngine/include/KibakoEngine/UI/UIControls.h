// UIControls.h - Declares basic UI controls built on top of UIElement.
#pragma once

#include <functional>
#include <string>
#include <string_view>

#include <DirectXMath.h>

#include "KibakoEngine/Fonts/Font.h"
#include "KibakoEngine/UI/UIElement.h"

namespace KibakoEngine {

    class UILabel : public UIElement
    {
    public:
        explicit UILabel(std::string name = "Label")
            : UIElement(std::move(name))
        {
        }

        void SetFont(const Font* font) { m_font = font; }
        void SetText(std::string text) { m_text = std::move(text); }
        void SetColor(const Color4& color) { m_color = color; }
        void SetScale(float scale) { m_scale = scale; }

        [[nodiscard]] const std::string& Text() const { return m_text; }

        void Render(SpriteBatch2D& batch, const UIContext& ctx) const override;

    private:
        const Font* m_font = nullptr;
        std::string m_text;
        Color4 m_color = Color4::White();
        float m_scale = 1.0f;
    };

    class UIImage : public UIElement
    {
    public:
        explicit UIImage(std::string name = "Image")
            : UIElement(std::move(name))
        {
        }

        void SetTexture(const Texture2D* texture) { m_texture = texture; }
        void SetColor(const Color4& color) { m_color = color; }

        void Render(SpriteBatch2D& batch, const UIContext& ctx) const override;

    private:
        const Texture2D* m_texture = nullptr;
        Color4 m_color = Color4::White();
    };

    class UIPanel : public UIElement
    {
    public:
        explicit UIPanel(std::string name = "Panel")
            : UIElement(std::move(name))
        {
        }

        void SetColor(const Color4& color) { m_color = color; }

        void Render(SpriteBatch2D& batch, const UIContext& ctx) const override;

    private:
        Color4 m_color = Color4{ 0.1f, 0.12f, 0.14f, 0.8f };
    };

    class UIButton : public UIElement
    {
    public:
        explicit UIButton(std::string name = "Button");

        void SetFont(const Font* font) { m_font = font; }
        void SetText(std::string text) { m_text = std::move(text); }
        void SetPadding(const DirectX::XMFLOAT2& padding) { m_padding = padding; }
        void SetTextScale(float scale) { m_textScale = scale; }
        void SetTextColor(const Color4& color) { m_textColor = color; }
        void SetNormalColor(const Color4& color) { m_colorNormal = color; }
        void SetHoverColor(const Color4& color) { m_colorHover = color; }
        void SetPressedColor(const Color4& color) { m_colorPressed = color; }
        void SetOnClick(std::function<void()> callback) { m_onClick = std::move(callback); }
        void SetCenterText(bool center) { m_centerText = center; }

        void Update(const UIContext& ctx) override;
        void Render(SpriteBatch2D& batch, const UIContext& ctx) const override;

    private:
        [[nodiscard]] bool HitTest(const UIContext& ctx) const;
        [[nodiscard]] Color4 CurrentColor() const;
        [[nodiscard]] DirectX::XMFLOAT2 TextPosition(const UIContext& ctx) const;
        [[nodiscard]] DirectX::XMFLOAT2 MeasureText() const;

        const Font* m_font = nullptr;
        std::string m_text;
        DirectX::XMFLOAT2 m_padding{ 12.0f, 10.0f };
        float m_textScale = 1.0f;
        bool m_centerText = true;

        Color4 m_textColor = Color4::White();
        Color4 m_colorNormal{ 0.13f, 0.15f, 0.18f, 0.92f };
        Color4 m_colorHover{ 0.18f, 0.2f, 0.23f, 0.95f };
        Color4 m_colorPressed{ 0.2f, 0.22f, 0.3f, 1.0f };

        bool m_hovered = false;
        bool m_pressed = false;
        bool m_trackingPress = false;
        std::function<void()> m_onClick;
    };

} // namespace KibakoEngine

