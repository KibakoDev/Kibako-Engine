// =====================================================
// Kibako2DEngine/Core/Input.h
// Handles keyboard, mouse, and text input via SDL2.
// =====================================================
#pragma once
#include <cstdint>
#include <SDL2/SDL.h>

namespace KibakoEngine {

    class Input {
    public:
        // Called at the start of each frame to reset transient states
        void BeginFrame();

        // Handle a single SDL event (keyboard, mouse, etc.)
        void HandleEvent(const SDL_Event& e);

        // Called at the end of each frame to store previous state
        void EndFrame();

        // Keyboard accessors
        inline bool KeyDown(SDL_Scancode sc) const { return m_keys && m_keys[sc] != 0; }
        inline bool KeyPressed(SDL_Scancode sc) const { return m_keys && m_keys[sc] && !m_prevKeys[sc]; }

        // Mouse accessors
        inline int MouseX() const { return m_mouseX; }
        inline int MouseY() const { return m_mouseY; }
        inline int WheelX() const { return m_wheelX; }
        inline int WheelY() const { return m_wheelY; }
        inline bool MouseDown(uint8_t btn) const { return (m_mouseButtons & SDL_BUTTON(btn)) != 0; }

        // Last typed character (ASCII, or low UTF-32)
        inline uint32_t TextChar() const { return m_textChar; }

    private:
        const uint8_t* m_keys = nullptr;
        uint8_t  m_prevKeysState[SDL_NUM_SCANCODES] = {};
        uint8_t  m_prevKeys[SDL_NUM_SCANCODES] = {};
        int      m_mouseX = 0;
        int      m_mouseY = 0;
        uint32_t m_mouseButtons = 0;
        int      m_wheelX = 0;
        int      m_wheelY = 0;
        uint32_t m_textChar = 0;
    };

}