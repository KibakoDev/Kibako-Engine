// =====================================================
// Kibako2DEngine/Core/Input.h
// SDL keyboard/mouse input helper (ASCII only).
// Usage per frame:
//   input.BeginFrame();
//   while (SDL_PollEvent(&e)) input.HandleEvent(e);
//   ... query KeyDown/KeyPressed/Mouse... ...
//   input.EndFrame();
// =====================================================

#pragma once
#include <cstdint>
#include <SDL2/SDL.h>  // brings SDL_Event, SDL_Scancode, SDL_BUTTON, SDL_NUM_SCANCODES

namespace KibakoEngine {

    class Input {
    public:
        Input();

        // Frame lifecycle
        void BeginFrame();                   // reset per-frame deltas (wheel, text), cache current SDL key ptr
        void HandleEvent(const SDL_Event&);  // feed every SDL event you receive
        void EndFrame();                     // snapshot current key/mouse states for "Pressed" queries

        // Keyboard queries
        bool KeyDown(SDL_Scancode sc) const;     // true while the key is held
        bool KeyPressed(SDL_Scancode sc) const;  // true only on the transition up->down for this frame

        // Mouse button queries (btn is 1..5, use SDL_BUTTON_LEFT, etc.)
        bool MouseDown(uint8_t btn) const;
        bool MousePressed(uint8_t btn) const;

        // Mouse position / wheel (accumulated deltas per frame)
        int  MouseX() const;
        int  MouseY() const;
        int  WheelX() const;
        int  WheelY() const;

        // ASCII-only last typed character this frame (0 if none)
        uint32_t TextChar() const;

    private:
        // Keyboard
        const uint8_t* m_keys = nullptr;                    // pointer from SDL_GetKeyboardState()
        uint8_t        m_prevKeyState[SDL_NUM_SCANCODES]{}; // snapshot from previous frame
        uint8_t        m_prevKeys[SDL_NUM_SCANCODES]{};     // optional mirror (kept for debugging/consistency)

        // Mouse
        int      m_mouseX = 0;
        int      m_mouseY = 0;
        int      m_wheelX = 0;  // per-frame delta
        int      m_wheelY = 0;  // per-frame delta
        uint32_t m_mouseButtons = 0;       // current SDL button bitfield
        uint32_t m_prevMouseButtons = 0;   // previous-frame copy

        // Text (ASCII only, first printable byte this frame)
        uint32_t m_textChar = 0;
    };

}