// Kibako2DEngine/include/KibakoEngine/Core/Layer.h
#pragma once

#include <string>

#include "KibakoEngine/Renderer/SpriteBatch2D.h"

namespace KibakoEngine {

    class Layer {
    public:
        explicit Layer(const char* name)
            : m_name(name ? name : "Layer")
        {
        }

        virtual ~Layer() = default;

        // Called when the layer is added to the Application
        virtual void OnAttach() {}

        // Called before removal (not used yet but ready if needed)
        virtual void OnDetach() {}

        // Per-frame update (game logic)
        virtual void OnUpdate(float dt)
        {
            (void)dt; // avoid unused parameter warning
        }

        // Per-frame render (submit sprites to batch)
        virtual void OnRender(SpriteBatch2D& batch)
        {
            (void)batch; // avoid unused parameter warning
        }

        const std::string& GetName() const { return m_name; }

    protected:
        std::string m_name;
    };

}