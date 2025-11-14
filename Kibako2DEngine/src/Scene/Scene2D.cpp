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

            const RectF& local = sprite.dst;

            // Compute scaled size (retain local dimensions)
            const float scaledWidth = local.w * e.transform.scale.x;
            const float scaledHeight = local.h * e.transform.scale.y;

            // Local offsets are expressed relative to the entity center
            const float offsetX = local.x * e.transform.scale.x;
            const float offsetY = local.y * e.transform.scale.y;

            RectF dst{};
            dst.w = scaledWidth;
            dst.h = scaledHeight;

            const float worldCenterX = e.transform.position.x + offsetX;
            const float worldCenterY = e.transform.position.y + offsetY;

            // Convert to top-left rectangle coordinates for rendering
            dst.x = worldCenterX - scaledWidth * 0.5f;
            dst.y = worldCenterY - scaledHeight * 0.5f;

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