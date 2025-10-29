// Kibako2DEngine/include/KibakoEngine/Core/Time.h
#pragma once
#include <cstdint>

namespace KibakoEngine {
    class Time {
    public:
        // Call once per frame
        void Tick();

        // Seconds
        inline double DeltaSeconds() const { return m_delta; }
        inline double TotalSeconds() const { return m_total; }
        inline double FPS() const { return m_delta > 0.0 ? 1.0 / m_delta : 0.0; }

    private:
        double  m_delta = 0.0;
        double  m_total = 0.0;
        uint64_t m_prevTicks = 0;
        bool    m_started = false;
    };
}