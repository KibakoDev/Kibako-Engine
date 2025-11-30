// Base interface for application layers
#pragma once

#include <string>

#include "KibakoEngine/Core/Debug.h"

namespace KibakoEngine {
    class SpriteBatch2D;
}

namespace KibakoEngine {

    class Layer
    {
    public:
        explicit Layer(const char* name)
            : m_name(name ? name : "Layer")
        {
        }

        virtual ~Layer() = default;

        virtual void OnAttach() {}
        virtual void OnDetach() {}

        virtual void OnUpdate(float dt)
        {
            KBK_UNUSED(dt);
        }

        virtual void OnRender(SpriteBatch2D& batch)
        {
            KBK_UNUSED(batch);
        }

        const std::string& GetName() const { return m_name; }

    protected:
        std::string m_name;
    };

} // namespace KibakoEngine

