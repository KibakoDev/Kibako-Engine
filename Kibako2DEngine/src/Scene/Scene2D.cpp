#include "KibakoEngine/Scene/Scene2D.h"

#include "KibakoEngine/Core/Debug.h"
#include "KibakoEngine/Core/Log.h"
#include "KibakoEngine/Renderer/SpriteBatch2D.h"

#include <algorithm>

namespace KibakoEngine {

    namespace
    {
        constexpr const char* kLogChannel = "Scene2D";
    }

    Entity2D& Scene2D::CreateEntity()
    {
        Entity2D& entity = m_entities.emplace_back();
        entity.id = m_nextID++;
        entity.active = true;

        KbkTrace(kLogChannel, "Created Entity2D id=%u", entity.id);

        return entity;
    }

    void Scene2D::DestroyEntity(EntityID id)
    {
        const auto it = std::find_if(m_entities.begin(), m_entities.end(), [id](const Entity2D& entity) {
            return entity.id == id;
        });
        if (it == m_entities.end())
            return;

        it->active = false;
        KbkTrace(kLogChannel, "Destroyed Entity2D id=%u (marked inactive)", id);
    }

    void Scene2D::Clear()
    {
        m_entities.clear();
        m_nextID = 1;
        KbkLog(kLogChannel, "Scene2D cleared");
    }

    Entity2D* Scene2D::FindEntity(EntityID id)
    {
        const auto it = std::find_if(m_entities.begin(), m_entities.end(), [id](const Entity2D& entity) {
            return entity.id == id;
        });
        return it != m_entities.end() ? &(*it) : nullptr;
    }

    const Entity2D* Scene2D::FindEntity(EntityID id) const
    {
        const auto it = std::find_if(m_entities.begin(), m_entities.end(), [id](const Entity2D& entity) {
            return entity.id == id;
        });
        return it != m_entities.end() ? &(*it) : nullptr;
    }

    void Scene2D::Update(float dt)
    {
        KBK_UNUSED(dt);
        // Pour l'instant, la scène ne gère aucune logique automatique.
        // Tout se fait dans le code gameplay (GameLayer / GameState).
    }

    void Scene2D::Render(SpriteBatch2D& batch) const
    {

        for (const auto& entity : m_entities) {
            if (!entity.active)
                continue;

            const auto& sprite = entity.sprite;
            if (!sprite.texture || !sprite.texture->IsValid())
                continue;

            const RectF& local = sprite.dst;

            // Compute scaled size (retain local dimensions)
            const float scaledWidth = local.w * entity.transform.scale.x;
            const float scaledHeight = local.h * entity.transform.scale.y;

            // Local offsets are expressed relative to the entity center
            const float offsetX = local.x * entity.transform.scale.x;
            const float offsetY = local.y * entity.transform.scale.y;

            RectF dst{};
            dst.w = scaledWidth;
            dst.h = scaledHeight;

            const float worldCenterX = entity.transform.position.x + offsetX;
            const float worldCenterY = entity.transform.position.y + offsetY;

            // Convert to top-left rectangle coordinates for rendering
            dst.x = worldCenterX - scaledWidth * 0.5f;
            dst.y = worldCenterY - scaledHeight * 0.5f;

            batch.Push(
                *sprite.texture,
                dst,
                sprite.src,
                sprite.color,
                entity.transform.rotation,
                sprite.layer
            );
        }
    }

} // namespace KibakoEngine