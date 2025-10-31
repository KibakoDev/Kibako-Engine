// =====================================================
// Kibako2DEngine/Core/Input.cpp
// Handles keyboard, mouse, and text input via SDL2.
// =====================================================

#include "KibakoEngine/Core/Input.h"

namespace KibakoEngine {

    void Input::BeginFrame() {
        m_wheelX = m_wheelY = 0;
        m_textChar = 0;
    }

    void Input::HandleEvent(const SDL_Event& e) {
        switch (e.type) {
        case SDL_MOUSEMOTION:
            m_mouseX = e.motion.x;
            m_mouseY = e.motion.y;
            break;

        case SDL_MOUSEBUTTONDOWN:
        case SDL_MOUSEBUTTONUP:
            m_mouseButtons = SDL_GetMouseState(&m_mouseX, &m_mouseY);
            break;

        case SDL_MOUSEWHEEL:
            m_wheelX += e.wheel.x;
            m_wheelY += e.wheel.y;
            break;

        case SDL_TEXTINPUT:
            m_textChar = static_cast<uint8_t>(e.text.text[0]); // ASCII only
            break;

        default:
            break;
        }
    }

    void Input::EndFrame() {
        // Update previous keyboard state for "pressed" detection
        if (!m_keys) {
            m_keys = SDL_GetKeyboardState(nullptr);
            for (int i = 0; i < SDL_NUM_SCANCODES; ++i)
                m_prevKeys[i] = m_keys[i];
            return;
        }

        for (int i = 0; i < SDL_NUM_SCANCODES; ++i)
            m_prevKeys[i] = m_prevKeysState[i];

        m_keys = SDL_GetKeyboardState(nullptr);

        for (int i = 0; i < SDL_NUM_SCANCODES; ++i)
            m_prevKeysState[i] = m_keys[i];
    }

}