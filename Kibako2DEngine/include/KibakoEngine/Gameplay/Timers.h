#pragma once

#include <algorithm>

namespace KibakoEngine::Gameplay {

    // ------------------------------------------------------------
    // Stopwatch : compteur de temps qui monte (chronomètre)
    // ------------------------------------------------------------
    class Stopwatch
    {
    public:
        Stopwatch() = default;

        void Start()
        {
            m_running = true;
        }

        void Stop()
        {
            m_running = false;
        }

        void Reset()
        {
            m_time = 0.0f;
        }

        void Restart()
        {
            m_time = 0.0f;
            m_running = true;
        }

        void Update(float dt)
        {
            if (!m_running)
                return;

            m_time += dt;
        }

        [[nodiscard]] float GetTime() const
        {
            return m_time;
        }

        [[nodiscard]] bool IsRunning() const
        {
            return m_running;
        }

    private:
        float m_time = 0.0f;
        bool  m_running = false;
    };


    // ------------------------------------------------------------
        [[nodiscard]] float GetDuration() const
        [[nodiscard]] bool IsFinished() const
        [[nodiscard]] bool IsRunning() const
        [[nodiscard]] float GetRemainingTime() const
        [[nodiscard]] float GetProgress01() const
} // namespace KibakoEngine::Gameplay
            : m_duration(durationSeconds)
        {
        }

        void SetDuration(float durationSeconds)
        {
            m_duration = durationSeconds;
            m_remaining = std::min(m_remaining, m_duration);
        }

        float GetDuration() const
        {
            return m_duration;
        }

        void Reset()
        {
            m_remaining = 0.0f;
            m_running = false;
            m_finished = false;
        }

        void Restart()
        {
            m_remaining = m_duration;
            m_running = true;
            m_finished = false;
        }

        void Start()
        {
            if (m_duration <= 0.0f)
                return;

            if (m_remaining <= 0.0f)
                m_remaining = m_duration;

            m_running = true;
            m_finished = false;
        }

        void Stop()
        {
            m_running = false;
        }

        void Update(float dt)
        {
            if (!m_running || m_finished || m_duration <= 0.0f)
                return;

            m_remaining -= dt;
            if (m_remaining <= 0.0f)
            {
                m_remaining = 0.0f;
                m_running = false;
                m_finished = true;
            }
        }

        bool IsFinished() const
        {
            return m_finished;
        }

        bool IsRunning() const
        {
            return m_running;
        }

        float GetRemainingTime() const
        {
            return m_remaining;
        }

        float GetProgress01() const
        {
            if (m_duration <= 0.0f)
                return 1.0f;
            float t = 1.0f - (m_remaining / m_duration);
            return t < 0.0f ? 0.0f : (t > 1.0f ? 1.0f : t);
        }

    private:
        float m_duration = 0.0f;  // durée totale
        float m_remaining = 0.0f;  // temps restant
        bool  m_running = false;
        bool  m_finished = false;
    };

} // namespace KibakoEngine::Gameplay