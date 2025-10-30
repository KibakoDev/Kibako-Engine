// Kibako2DEngine/src/Core/Input.cpp
#include "KibakoEngine/Core/Input.h"

namespace KibakoEngine {

    void Input::BeginFrame() {
        m_wheelX = m_wheelY = 0;
        m_textChar = 0;

        // First-time init: hook SDL keyboard pointer and seed snapshots
        if (!m_keys) {
            m_keys = SDL_GetKeyboardState(nullptr);
            for (int i = 0; i < SDL_NUM_SCANCODES; ++i) {
                m_prevKeysState[i] = m_keys[i];
                m_prevKeys[i] = m_keys[i];
            }
        }
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
            m_textChar = static_cast<uint8_t>(e.text.text[0]); // simple ASCII
            break;
        default: break;
        }
    }

    void Input::EndFrame() {
        // Copy previous snapshot
        for (int i = 0; i < SDL_NUM_SCANCODES; ++i)
            m_prevKeys[i] = m_prevKeysState[i];

        // Refresh current keyboard state (SDL keeps a stable pointer)
        m_keys = SDL_GetKeyboardState(nullptr);

        // Store snapshot for next frame
        for (int i = 0; i < SDL_NUM_SCANCODES; ++i)
            m_prevKeysState[i] = m_keys[i];
    }
}