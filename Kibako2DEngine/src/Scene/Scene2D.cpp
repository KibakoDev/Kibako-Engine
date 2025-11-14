#include "KibakoEngine/Scene/Scene2D.h"

#include "KibakoEngine/Core/Debug.h"
#include "KibakoEngine/Core/Log.h"
#include "KibakoEngine/Renderer/SpriteBatch2D.h"

namespace KibakoEngine {

    namespace
    {
        constexpr const char* kLogChannel = "Scene2D";
    }

    Entity2D& Scene2D::CreateEntity()
    {
        Entity2D entity;
        entity.id = m_nextID++;
        entity.active = true;

        m_entities.push_back(entity);

        KbkTrace(kLogChannel, "Created Entity2D id=%u", entity.id);

        return m_entities.back();
    }

    void Scene2D::DestroyEntity(EntityID id)
    {
        for (auto& e : m_entities) {
            if (e.id == id) {
                e.active = false;
                KbkTrace(kLogChannel, "Destroyed Entity2D id=%u (marked inactive)", id);
                return;
            }
        }
    }

    void Scene2D::Clear()
    {
        m_entities.clear();
        KbkLog(kLogChannel, "Scene2D cleared");
    }

    Entity2D* Scene2D::FindEntity(EntityID id)
    {
        for (auto& e : m_entities) {
            if (e.id == id)
                return &e;
        }
        return nullptr;
    }

    const Entity2D* Scene2D::FindEntity(EntityID id) const
    {
        for (const auto& e : m_entities) {
            if (e.id == id)
                return &e;
        }
        return nullptr;
    }

    void Scene2D::Update(float dt)
    {
        KBK_UNUSED(dt);
        // Pour l'instant, la scène ne gère aucune logique automatique.
        // Tout se fait dans le code gameplay (GameLayer / GameState).
    }

    void Scene2D::Render(SpriteBatch2D& batch) const
    {

        for (const auto& e : m_entities) {
            if (!e.active)
                continue;

            const auto& sprite = e.sprite;
            if (!sprite.texture || !sprite.texture->IsValid())
                continue;

            RectF dst = sprite.dst;

            // Apply scale
            dst.w *= e.transform.scale.x;
            dst.h *= e.transform.scale.y;

            // Apply position
            dst.x = e.transform.position.x + sprite.dst.x;
            dst.y = e.transform.position.y + sprite.dst.y;

            batch.Push(
                *sprite.texture,
                dst,
                sprite.src,
                sprite.color,
                e.transform.rotation,
                sprite.layer
            );
        }
    }

} // namespace KibakoEngine