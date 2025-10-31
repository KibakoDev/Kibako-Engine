// Kibako2DSandbox/src/main.cpp
#define SDL_MAIN_HANDLED
#include "KibakoEngine/Core/Application.h"
#include <iostream>

int main() {
    KibakoEngine::Application app;
    if (!app.Init(1280, 720, "Kibako Sandbox")) {
        std::cerr << "Failed to initialize Kibako Engine Application\n";
        return -1;
    }
    app.Run();
    app.Shutdown();
    return 0;
}