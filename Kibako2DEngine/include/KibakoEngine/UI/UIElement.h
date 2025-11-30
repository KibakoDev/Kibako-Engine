// Minimal UI hierarchy and screen manager
#pragma once

#include <memory>
#include <optional>
#include <string>
#include <vector>

#include <DirectXMath.h>

#include "KibakoEngine/Core/Input.h"
#include "KibakoEngine/Renderer/SpriteTypes.h"
#include "KibakoEngine/UI/UIStyle.h"

namespace KibakoEngine {
    class SpriteBatch2D;

    enum class UIAnchor
    {
        TopLeft,
        TopRight,
        BottomLeft,
        BottomRight,
        Center,
    };

    struct UIContext
    {
        DirectX::XMFLOAT2 screenSize{ 0.0f, 0.0f };
        const Input* input = nullptr;
        float deltaTime = 0.0f;
    };

    class UIElement
    {
    public:
        explicit UIElement(std::string name = "");
        virtual ~UIElement() = default;

        void SetPosition(const DirectX::XMFLOAT2& position) { m_position = position; InvalidateLayout(); }
        void SetSize(const DirectX::XMFLOAT2& size) { m_size = size; InvalidateLayout(); }
        void SetAnchor(UIAnchor anchor) { m_anchor = anchor; InvalidateLayout(); }
        void SetVisible(bool visible) { m_visible = visible; }
        void SetLayer(int layer) { m_layer = layer; }
        void SetColorTint(const Color4& color) { m_tint = color; }
        void ClearColorTint() { m_tint.reset(); }

        [[nodiscard]] const std::string& Name() const { return m_name; }
        [[nodiscard]] const DirectX::XMFLOAT2& Position() const { return m_position; }
        [[nodiscard]] const DirectX::XMFLOAT2& Size() const { return m_size; }
        [[nodiscard]] bool Visible() const { return m_visible; }
        [[nodiscard]] bool IsVisible() const { return m_visible; }
        [[nodiscard]] int Layer() const { return m_layer; }
        [[nodiscard]] const std::optional<Color4>& ColorTint() const { return m_tint; }

        [[nodiscard]] const DirectX::XMFLOAT2& WorldPosition(const UIContext& ctx) const;
        [[nodiscard]] const RectF& WorldRect(const UIContext& ctx) const;

        virtual void Update(const UIContext& ctx);
        virtual void Render(SpriteBatch2D& batch, const UIContext& ctx, const UIStyle& style) const;

        void AddChild(std::unique_ptr<UIElement> child);

        template<typename T, typename... Args>
        T& EmplaceChild(Args&&... args)
        {
            auto element = std::make_unique<T>(std::forward<Args>(args)...);
            T& ref = *element;
            AddChild(std::move(element));
            return ref;
        }

    protected:
        void InvalidateLayout();
        void UpdateLayout(const UIContext& ctx) const;
        virtual void OnUpdate(const UIContext&) {}
        virtual void OnRender(SpriteBatch2D&, const UIContext&, const UIStyle&) const {}

        [[nodiscard]] DirectX::XMFLOAT2 ParentSize(const UIContext& ctx) const;
        [[nodiscard]] DirectX::XMFLOAT2 AnchorOffset(const UIContext& ctx) const;

        std::string m_name;
        DirectX::XMFLOAT2 m_position{ 0.0f, 0.0f };
        DirectX::XMFLOAT2 m_size{ 0.0f, 0.0f };
        UIAnchor m_anchor = UIAnchor::TopLeft;
        bool m_visible = true;
        int m_layer = 10000;
        std::optional<Color4> m_tint;

        mutable bool m_layoutDirty = true;
        mutable DirectX::XMFLOAT2 m_cachedWorldPosition{ 0.0f, 0.0f };
        mutable RectF m_cachedWorldRect;
        mutable DirectX::XMFLOAT2 m_cachedScreenSize{ 0.0f, 0.0f };

        UIElement* m_parent = nullptr;
        std::vector<std::unique_ptr<UIElement>> m_children;
    };

    class UIScreen
    {
    public:
        explicit UIScreen(std::string name = "");
        virtual ~UIScreen() = default;

        void SetVisible(bool visible) { m_visible = visible; }
        [[nodiscard]] bool Visible() const { return m_visible; }
        [[nodiscard]] bool IsVisible() const { return m_visible; }
        [[nodiscard]] const std::string& Name() const { return m_name; }

        [[nodiscard]] UIElement& Root() { return m_root; }
        [[nodiscard]] const UIElement& Root() const { return m_root; }

        virtual void OnUpdate(const UIContext& ctx);
        virtual void OnRender(SpriteBatch2D& batch, const UIContext& ctx, const UIStyle& style) const;

    private:
        std::string m_name;
        UIElement m_root{ "Root" };
        bool m_visible = true;
    };

    class UISystem
    {
    public:
        UISystem();

        void SetScreenSize(float width, float height) { m_screenSize = DirectX::XMFLOAT2{ width, height }; }
        void SetInput(const Input* input) { m_input = input; }

        [[nodiscard]] UIStyle& Style() { return m_style; }
        [[nodiscard]] const UIStyle& Style() const { return m_style; }
        void SetStyle(const UIStyle& style) { m_style = style; }

        void PushScreen(std::unique_ptr<UIScreen> screen);
        UIScreen& CreateScreen(const std::string& name = "");
        void PopScreen();
        void Clear();

        void Update(float dt);
        void Render(SpriteBatch2D& batch) const;

    private:
        DirectX::XMFLOAT2 m_screenSize{ 0.0f, 0.0f };
        const Input* m_input = nullptr;
        float m_lastDeltaTime = 0.0f;
        UIStyle m_style{};
        std::vector<std::unique_ptr<UIScreen>> m_screens;
    };

} // namespace KibakoEngine

