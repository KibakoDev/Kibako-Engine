// Kibako2DEngine/include/KibakoEngine/Core/Input.h
#pragma once
#include <cstdint>
#include <SDL2/SDL.h>

namespace KibakoEngine {
    class Input {
    public:
        // Call once per frame, before reading state
        void BeginFrame() { m_wheelX = m_wheelY = 0; m_textChar = 0; }

        // Keyboard
        inline bool KeyDown(SDL_Scancode sc) const { return m_keys[sc] != 0; }
        inline bool KeyPressed(SDL_Scancode sc) const { return m_keys[sc] && !m_prevKeys[sc]; }

        // Mouse
        inline int MouseX() const { return m_mouseX; }
        inline int MouseY() const { return m_mouseY; }
        inline int WheelX() const { return m_wheelX; }
        inline int WheelY() const { return m_wheelY; }
        inline bool MouseDown(uint8_t btn) const { return (m_mouseButtons & SDL_BUTTON(btn)) != 0; }

        // Optional: last text input (ASCII/UTF-32 low)
        inline uint32_t TextChar() const { return m_textChar; }

        // Feed SDL events here
        void HandleEvent(const SDL_Event& e);

        // Finalize (copy current keys to prev)
        void EndFrame();

    private:
        const uint8_t* m_keys = nullptr;
        uint8_t        m_prevKeysState[SDL_NUM_SCANCODES] = {};
        uint8_t        m_prevKeys[SDL_NUM_SCANCODES] = {};
        int            m_mouseX = 0, m_mouseY = 0;
        uint32_t       m_mouseButtons = 0;
        int            m_wheelX = 0, m_wheelY = 0;
        uint32_t       m_textChar = 0;
    };
}