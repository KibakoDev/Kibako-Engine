// Kibako2DSandbox/src/main.cpp
#include "KibakoEngine/Core/Application.h"
#include <iostream>

int main()
{
    KibakoEngine::Application app;

    if (!app.Init(1280, 720, "Kibako Sandbox"))
    {
        std::cerr << "Failed to initialize Kibako Application\n";
        return -1;
    }

    app.Run();
    app.Shutdown();
    return 0;
}
