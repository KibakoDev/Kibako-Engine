// Centralized styling for UI controls
#pragma once

#include <DirectXMath.h>

#include "KibakoEngine/Fonts/Font.h"
#include "KibakoEngine/Renderer/SpriteTypes.h"

namespace KibakoEngine {

    class UILabel;
    class UIPanel;
    class UIButton;
    class UISlider;

    struct UIStyle
    {
        const Font* font = nullptr;

        Color4 headingColor{ 1.0f, 0.96f, 0.7f, 1.0f };
        Color4 primaryTextColor = Color4::White();
        Color4 mutedTextColor{ 0.8f, 0.88f, 1.0f, 1.0f };
        Color4 panelColor{ 0.08f, 0.09f, 0.13f, 0.94f };
        Color4 buttonNormal{ 0.15f, 0.15f, 0.15f, 0.92f };
        Color4 buttonHover{ 0.25f, 0.25f, 0.25f, 0.95f };
        Color4 buttonPressed{ 0.3f, 0.3f, 0.3f, 1.0f };

        Color4 sliderTrack{ 0.12f, 0.13f, 0.16f, 0.9f };
        Color4 sliderFill{ 0.27f, 0.6f, 1.0f, 0.95f };
        Color4 sliderHandle{ 0.9f, 0.9f, 0.9f, 1.0f };
        Color4 sliderHandleHover{ 1.0f, 1.0f, 1.0f, 1.0f };

        DirectX::XMFLOAT2 buttonSize{ 480.0f, 48.0f };
        DirectX::XMFLOAT2 buttonPadding{ 100.0f, 11.0f };

        float sliderHeight = 32.0f;
        float sliderTrackHeight = 12.0f;
        float sliderHandleWidth = 18.0f;

        float headingScale = 1.05f;
        float bodyScale = 0.9f;
        float captionScale = 0.75f;
        float buttonTextScale = 0.4f;

        void ApplyHeading(UILabel& label) const;
        void ApplyBody(UILabel& label) const;
        void ApplyCaption(UILabel& label) const;
        void ApplyPanel(UIPanel& panel) const;
        void ApplyButton(UIButton& button) const;
        void ApplySlider(UISlider& slider) const;
    };

} // namespace KibakoEngine

