// =====================================================
// Kibako2DEngine/Core/Time.h
// High-precision time tracking using SDL performance counters.
// =====================================================
#pragma once
#include <cstdint>

namespace KibakoEngine {

    class Time {
    public:
        // Called once per frame to update delta time
        void Tick();

        // Frame delta in seconds
        inline double DeltaSeconds() const { return m_delta; }

        // Total elapsed time since start (seconds)
        inline double TotalSeconds() const { return m_total; }

        // Frames per second
        inline double FPS() const { return m_delta > 0.0 ? 1.0 / m_delta : 0.0; }

    private:
        double   m_delta = 0.0;
        double   m_total = 0.0;
        uint64_t m_prevTicks = 0;
        bool     m_started = false;
    };

}