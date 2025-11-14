#pragma once

namespace KibakoEngine {

    // On dclare juste qu'il existe un Transform2D dans ce namespace.
    // La dfinition complte est dans Scene2D.h.
    struct Transform2D;
    class Input;
    class SpriteBatch2D;

    // --------------------------
    // ---- Collider Types ------
    // --------------------------

    struct CircleCollider2D
    {
        float radius = 0.0f;
        bool  active = true;
    };

    struct AABBCollider2D
    {
        float halfW = 0.0f;
        float halfH = 0.0f;
        bool  active = true;
    };

    // Composant de collision attachable  une entit 2D
    struct CollisionComponent2D
    {
        CircleCollider2D* circle = nullptr;
        AABBCollider2D* aabb = nullptr;
    };

    // --------------------------------------
    // ----------- Collision Tests ----------
    // --------------------------------------

    bool Intersects(const CircleCollider2D& c1, const Transform2D& t1,
        const CircleCollider2D& c2, const Transform2D& t2);

    bool Intersects(const AABBCollider2D& b1, const Transform2D& t1,
        const AABBCollider2D& b2, const Transform2D& t2);

#if KBK_DEBUG_BUILD
    namespace CollisionDebug2D
    {
        void UpdateToggle(const Input& input);
        void BeginFrame();
        void RenderDebug(SpriteBatch2D& batch);
    }
#endif

} // namespace KibakoEngine
