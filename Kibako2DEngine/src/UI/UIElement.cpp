// UIElement.cpp - Implements basic UI containers and screen management.
#include "KibakoEngine/UI/UIElement.h"

#include "KibakoEngine/Renderer/SpriteBatch2D.h"

namespace {
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

    DirectX::XMFLOAT2 UIElement::WorldPosition(const UIContext& ctx) const
    {
        DirectX::XMFLOAT2 pos = AnchorOffset(ctx);
        pos = Add(pos, m_position);

        if (m_parent)
            pos = Add(pos, m_parent->WorldPosition(ctx));

        return pos;
    }

    RectF UIElement::WorldRect(const UIContext& ctx) const
    {
        const DirectX::XMFLOAT2 pos = WorldPosition(ctx);
        return RectF::FromXYWH(pos.x, pos.y, m_size.x, m_size.y);
    }

    void UIElement::Update(const UIContext& ctx)
    {
        if (!m_visible)
            return;

        for (auto& child : m_children)
            child->Update(ctx);
    }

    void UIElement::Render(SpriteBatch2D& batch, const UIContext& ctx) const
    {
        if (!m_visible)
            return;

        for (const auto& child : m_children)
            child->Render(batch, ctx);
    }

    void UIElement::AddChild(std::unique_ptr<UIElement> child)
    {
        if (!child)
            return;

        child->m_parent = this;
        m_children.push_back(std::move(child));
    }

    void UIScreen::OnUpdate(const UIContext& ctx)
    {
        if (!m_visible)
            return;

        m_root.SetSize(ctx.screenSize);
        m_root.Update(ctx);
    }

    void UIScreen::OnRender(SpriteBatch2D& batch, const UIContext& ctx) const
    {
        if (!m_visible)
            return;

        m_root.Render(batch, ctx);
    }

    void UISystem::PushScreen(std::unique_ptr<UIScreen> screen)
    {
        if (screen)
            m_screens.push_back(std::move(screen));
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
            screen->OnRender(batch, ctx);
    }

} // namespace KibakoEngine

