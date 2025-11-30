// UI containers and screen management
#include "KibakoEngine/UI/UIElement.h"

#include "KibakoEngine/Renderer/SpriteBatch2D.h"

namespace
{
    DirectX::XMFLOAT2 Add(const DirectX::XMFLOAT2& a, const DirectX::XMFLOAT2& b)
    {
        return DirectX::XMFLOAT2{ a.x + b.x, a.y + b.y };
    }
}

namespace KibakoEngine {

    UIElement::UIElement(std::string name)
        : m_name(std::move(name))
    {
    }

    DirectX::XMFLOAT2 UIElement::ParentSize(const UIContext& ctx) const
    {
        if (m_parent)
            return m_parent->Size();

        return ctx.screenSize;
    }

    void UIElement::InvalidateLayout()
    {
        m_layoutDirty = true;
        for (auto& child : m_children)
            child->InvalidateLayout();
    }

    DirectX::XMFLOAT2 UIElement::AnchorOffset(const UIContext& ctx) const
    {
        const DirectX::XMFLOAT2 parentSize = ParentSize(ctx);
        DirectX::XMFLOAT2 offset{ 0.0f, 0.0f };

        switch (m_anchor) {
        case UIAnchor::TopLeft: break;
        case UIAnchor::TopRight:
            offset.x = parentSize.x - m_size.x;
            break;
        case UIAnchor::BottomLeft:
            offset.y = parentSize.y - m_size.y;
            break;
        case UIAnchor::BottomRight:
            offset.x = parentSize.x - m_size.x;
            offset.y = parentSize.y - m_size.y;
            break;
        case UIAnchor::Center:
            offset.x = 0.5f * (parentSize.x - m_size.x);
            offset.y = 0.5f * (parentSize.y - m_size.y);
            break;
        }

        return offset;
    }

    void UIElement::UpdateLayout(const UIContext& ctx) const
    {
        if (!m_layoutDirty && m_cachedScreenSize.x == ctx.screenSize.x && m_cachedScreenSize.y == ctx.screenSize.y)
            return;

        DirectX::XMFLOAT2 pos = AnchorOffset(ctx);
        pos = Add(pos, m_position);

        if (m_parent)
            pos = Add(pos, m_parent->WorldPosition(ctx));

        m_cachedWorldPosition = pos;
        m_cachedWorldRect = RectF::FromXYWH(pos.x, pos.y, m_size.x, m_size.y);
        m_cachedScreenSize = ctx.screenSize;
        m_layoutDirty = false;
    }

    const DirectX::XMFLOAT2& UIElement::WorldPosition(const UIContext& ctx) const
    {
        UpdateLayout(ctx);
        return m_cachedWorldPosition;
    }

    const RectF& UIElement::WorldRect(const UIContext& ctx) const
    {
        UpdateLayout(ctx);
        return m_cachedWorldRect;
    }

    void UIElement::Update(const UIContext& ctx)
    {
        if (!m_visible)
            return;

        UpdateLayout(ctx);
        OnUpdate(ctx);

        for (auto& child : m_children)
            child->Update(ctx);
    }

    void UIElement::Render(SpriteBatch2D& batch, const UIContext& ctx, const UIStyle& style) const
    {
        if (!m_visible)
            return;

        UpdateLayout(ctx);
        OnRender(batch, ctx, style);

        for (const auto& child : m_children)
            child->Render(batch, ctx, style);
    }

    void UIElement::AddChild(std::unique_ptr<UIElement> child)
    {
        if (!child)
            return;

        child->m_parent = this;
        child->InvalidateLayout();
        m_children.push_back(std::move(child));
    }

    UIScreen::UIScreen(std::string name)
        : m_name(std::move(name))
    {
    }

    void UIScreen::OnUpdate(const UIContext& ctx)
    {
        if (!m_visible)
            return;

        m_root.SetSize(ctx.screenSize);
        m_root.Update(ctx);
    }

    void UIScreen::OnRender(SpriteBatch2D& batch, const UIContext& ctx, const UIStyle& style) const
    {
        if (!m_visible)
            return;

        m_root.Render(batch, ctx, style);
    }

    UISystem::UISystem() = default;

    void UISystem::PushScreen(std::unique_ptr<UIScreen> screen)
    {
        if (screen)
            m_screens.push_back(std::move(screen));
    }

    UIScreen& UISystem::CreateScreen(const std::string& name)
    {
        auto screen = std::make_unique<UIScreen>(name);
        UIScreen& ref = *screen;
        m_screens.push_back(std::move(screen));
        return ref;
    }

    void UISystem::PopScreen()
    {
        if (!m_screens.empty())
            m_screens.pop_back();
    }

    void UISystem::Clear()
    {
        m_screens.clear();
    }

    void UISystem::Update(float dt)
    {
        m_lastDeltaTime = dt;

        const UIContext ctx{ m_screenSize, m_input, dt };
        for (auto& screen : m_screens)
            screen->OnUpdate(ctx);
    }

    void UISystem::Render(SpriteBatch2D& batch) const
    {
        const UIContext ctx{ m_screenSize, m_input, m_lastDeltaTime };
        for (const auto& screen : m_screens)
            screen->OnRender(batch, ctx, m_style);
    }

} // namespace KibakoEngine

