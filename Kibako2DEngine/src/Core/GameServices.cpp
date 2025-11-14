#include "KibakoEngine/Core/GameServices.h"

#include "KibakoEngine/Core/Debug.h"
#include "KibakoEngine/Core/Log.h"

namespace KibakoEngine::GameServices {

    namespace
    {
        GameTime g_time{};
        bool     g_initialized = false;

        constexpr const char* kLogChannel = "GameServices";
    }

    void Init()
    {
        g_time = GameTime{};
        g_time.timeScale = 1.0;
        g_time.paused = false;
        g_initialized = true;

        KbkLog(kLogChannel, "GameServices initialized");
    }

    void Shutdown()
    {
        if (!g_initialized)
            return;

        g_initialized = false;
        KbkLog(kLogChannel, "GameServices shutdown");
    }

    void Update(double rawDeltaSeconds)
    {
        if (!g_initialized)
            Init(); // sécurité, au cas où

        if (rawDeltaSeconds < 0.0)
            rawDeltaSeconds = 0.0;

        g_time.rawDeltaSeconds = rawDeltaSeconds;
        g_time.totalRawSeconds += rawDeltaSeconds;

        if (g_time.paused || g_time.timeScale <= 0.0) {
            g_time.scaledDeltaSeconds = 0.0;
            // même en pause, on pourrait choisir d'accumuler totalScaled ou pas.
            return;
        }

        const double scaledDt = rawDeltaSeconds * g_time.timeScale;
        g_time.scaledDeltaSeconds = scaledDt;
        g_time.totalScaledSeconds += scaledDt;
    }

    const GameTime& GetTime()
    {
        return g_time;
    }

    void SetTimeScale(double scale)
    {
        if (scale < 0.0)
            scale = 0.0;
        g_time.timeScale = scale;
    }

    double GetTimeScale()
    {
        return g_time.timeScale;
    }

    void SetPaused(bool paused)
    {
        g_time.paused = paused;
    }

    bool IsPaused()
    {
        return g_time.paused;
    }

    void TogglePause()
    {
        g_time.paused = !g_time.paused;
    }

} // namespace KibakoEngine::GameServices