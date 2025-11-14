#pragma once

#include <cstdint>
#include <vector>

#include <DirectXMath.h>

#include "KibakoEngine/Renderer/SpriteTypes.h"
#include "KibakoEngine/Renderer/Texture2D.h"
#include "KibakoEngine/Collision/Collision2D.h"

namespace KibakoEngine {

    class SpriteBatch2D;

    using EntityID = std::uint32_t;

    struct Transform2D
    {
        DirectX::XMFLOAT2 position{ 0.0f, 0.0f }; // world-space center
        float              rotation = 0.0f;       // radians
        DirectX::XMFLOAT2  scale{ 1.0f, 1.0f };
    };

    struct SpriteRenderer2D
    {
        Texture2D* texture = nullptr;

        RectF  dst{ 0.0f, 0.0f, 0.0f, 0.0f };        // local size and relative offset from center
        RectF  src{ 0.0f, 0.0f, 1.0f, 1.0f };        // UV [0..1]
        Color4 color = Color4::White();
        int    layer = 0;
    };

    struct Entity2D
    {
        EntityID id = 0;
        bool     active = true;

        Transform2D        transform;
        SpriteRenderer2D   sprite;
        CollisionComponent2D collision;
    };

    class Scene2D
    {
    public:
        Scene2D() = default;

        // Creation / destruction
        [[nodiscard]] Entity2D& CreateEntity();
        void                   DestroyEntity(EntityID id); // marks entity as inactive

        void Clear(); // removes all entities

        // Lookup
        [[nodiscard]] Entity2D*       FindEntity(EntityID id);
        [[nodiscard]] const Entity2D* FindEntity(EntityID id) const;

        std::vector<Entity2D>& Entities() { return m_entities; }
        const std::vector<Entity2D>& Entities() const { return m_entities; }

        // Placeholder update (no internal logic yet)
        void Update(float dt);

        // Render every active entity that has a valid sprite
        void Render(SpriteBatch2D& batch) const;

    private:
        EntityID m_nextID = 1;
        std::vector<Entity2D> m_entities;
    };

} // namespace KibakoEngine
