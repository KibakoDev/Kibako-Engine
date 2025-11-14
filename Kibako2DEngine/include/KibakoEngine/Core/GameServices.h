#pragma once

#include <cstdint>

namespace KibakoEngine {

    // État global du temps de jeu
    struct GameTime
    {
        double rawDeltaSeconds = 0.0;  // dt brut (temps réel)
        double scaledDeltaSeconds = 0.0;  // dt après timeScale / pause

        double totalRawSeconds = 0.0;  // temps cumulé brut
        double totalScaledSeconds = 0.0;  // temps cumulé avec scaling

        double timeScale = 1.0;  // 1.0 = normal, 0.5 = ralenti, 2.0 = accéléré
        bool   paused = false;
    };

    namespace GameServices
    {
        // À appeler une fois au démarrage (si besoin de reset explicite)
        void Init();
        void Shutdown();

        // À appeler une fois par frame avec le dt BRUT (non-scalé)
        void Update(double rawDeltaSeconds);

        // Accès lecture seule
        const GameTime& GetTime();

        // Helpers
        inline double GetScaledDeltaTime()
        {
            return GetTime().scaledDeltaSeconds;
        }

        inline double GetRawDeltaTime()
        {
            return GetTime().rawDeltaSeconds;
        }

        // Contrôle du time scale
        void   SetTimeScale(double scale);
        double GetTimeScale();

        // Pause globale
        void SetPaused(bool paused);
        bool IsPaused();
        void TogglePause();
    }

} // namespace KibakoEngine