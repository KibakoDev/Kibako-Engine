// SDL timing helper
#include "KibakoEngine/Core/Time.h"

#include <SDL2/SDL.h>

namespace KibakoEngine {

    void Time::Tick()
    {
        const uint64_t now = SDL_GetPerformanceCounter();
        const double   freq = static_cast<double>(SDL_GetPerformanceFrequency());

        if (!m_started) {
            m_started = true;
            m_prevTicks = now;
            m_delta = 0.0;
            m_total = 0.0;
            return;
        }

        m_delta = static_cast<double>(now - m_prevTicks) / freq;
        m_prevTicks = now;
        m_total += m_delta;
    }

} // namespace KibakoEngine

