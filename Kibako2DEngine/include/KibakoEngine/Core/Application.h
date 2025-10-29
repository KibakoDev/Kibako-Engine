// Kibako2DEngine/include/KibakoEngine/Core/Application.h
#pragma once

#include <cstdint>

namespace KibakoEngine
{
    class Application
    {
    public:
        // Window and D3D11 initialisation
        bool Init(int width, int height, const char* title);

        // Main loop (events + clear + present)
        void Run();

        // Cleanup
        void Shutdown();

    private:
        // Hides SDL/D3D11 dependencies
        struct Impl;
        Impl* m_impl = nullptr;
    };
}