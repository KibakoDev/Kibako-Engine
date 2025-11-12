// Kibako2DEngine/Core/Input.cpp
// Simple SDL input helper with per-frame state.

#include "KibakoEngine/Core/Input.h"
#include <SDL2/SDL.h>
#include <cstring>   // memcpy
#include <cstdint>

namespace KibakoEngine {

    Input::Input()
    {
        // SDL returns a persistent pointer to its internal keyboard state.
        // It becomes valid after the first PumpEvents / PollEvent.
        m_keys = SDL_GetKeyboardState(nullptr);

        std::memset(m_prevKeyState, 0, sizeof(m_prevKeyState));
        m_mouseX = m_mouseY = 0;
        m_wheelX = m_wheelY = 0;
        m_mouseButtons = 0;
        m_prevMouseButtons = 0;
        m_textChar = 0;
    }

    void Input::BeginFrame()
    {
        // Clear per-frame deltas
        m_wheelX = 0;
        m_wheelY = 0;
        m_textChar = 0;

        // Snapshot mouse buttons for "Pressed" detection
        m_prevMouseButtons = m_mouseButtons;

        // Cache the pointer (SDL returns the same pointer each time, but
        // we re-fetch for clarity).
        m_keys = SDL_GetKeyboardState(nullptr);
    }

    void Input::HandleEvent(const SDL_Event& e)
    {
        switch (e.type)
        {
        case SDL_MOUSEMOTION:
            m_mouseX = e.motion.x;
            m_mouseY = e.motion.y;
            break;

        case SDL_MOUSEWHEEL:
            // SDL mouse wheel is small ints; accumulate for this frame
            // Note: if you need natural/inverted, flip the sign here.
            m_wheelX += e.wheel.x;
            m_wheelY += e.wheel.y;
            break;

        case SDL_MOUSEBUTTONDOWN:
            m_mouseButtons |= SDL_BUTTON(e.button.button);
            break;

        case SDL_MOUSEBUTTONUP:
            m_mouseButtons &= ~SDL_BUTTON(e.button.button);
            break;

        case SDL_TEXTINPUT:
            // ASCII only (no Unicode like requested). Grab first byte if printable.
            if (e.text.text[0] >= 32 && e.text.text[0] <= 126)
                m_textChar = static_cast<uint32_t>(static_cast<unsigned char>(e.text.text[0]));
            break;

            // Keyboard: we rely on SDL_GetKeyboardState(), which gets updated by PollEvent/PumpEvents.
            // No per-key bookkeeping needed here.
        default:
            break;
        }
    }

    void Input::EndFrame()
    {
        // Take a snapshot of the current keyboard state for "KeyPressed" next frame.
        // SDL_NUM_SCANCODES is the maximum; SDL guarantees m_keys has that many bytes.
        std::memcpy(m_prevKeyState, m_keys, SDL_NUM_SCANCODES);

    }

    // ----------------------
    // Convenience getters
    // ----------------------
    bool Input::KeyDown(SDL_Scancode sc) const
    {
        return m_keys && m_keys[sc] != 0;
    }

    bool Input::KeyPressed(SDL_Scancode sc) const
    {
        // true if currently down but was up last frame
        const uint8_t now = m_keys ? m_keys[sc] : 0;
        const uint8_t prev = m_prevKeyState[sc];
        return (now != 0) && (prev == 0);
    }

    bool Input::MouseDown(uint8_t btn) const
    {
        return (m_mouseButtons & SDL_BUTTON(btn)) != 0;
    }

    bool Input::MousePressed(uint8_t btn) const
    {
        const uint32_t mask = SDL_BUTTON(btn);
        return ((m_mouseButtons & mask) != 0) && ((m_prevMouseButtons & mask) == 0);
    }

    int Input::MouseX() const { return m_mouseX; }
    int Input::MouseY() const { return m_mouseY; }
    int Input::WheelX() const { return m_wheelX; }
    int Input::WheelY() const { return m_wheelY; }
    uint32_t Input::TextChar() const { return m_textChar; }

}