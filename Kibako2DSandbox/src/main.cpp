#define SDL_MAIN_HANDLED
#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <d3d11.h>
#include <dxgi.h>
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")

#include <SDL2/SDL.h>
#include <SDL2/SDL_syswm.h>
#include <stdexcept>
#include <iostream>

// Simple RAII
template<typename T> inline void SafeRelease(T*& p) { if (p) { p->Release(); p = nullptr; } }

// D3D Context
struct D3DContext
{
    ID3D11Device* device = nullptr;
    ID3D11DeviceContext* context = nullptr;
    IDXGISwapChain* swapChain = nullptr;
    ID3D11RenderTargetView* rtv = nullptr;
};

// Create D3D11
static void CreateD3D11(SDL_Window* window, D3DContext& out)
{
    // 1) Get HWND from SDL
    SDL_SysWMinfo wminfo;
    SDL_VERSION(&wminfo.version);
    if (!SDL_GetWindowWMInfo(window, &wminfo))
        throw std::runtime_error("SDL_GetWindowWMInfo failed");

    HWND hwnd = wminfo.info.win.window; // HWND result

    // 2) Setting up Swap Chain
    DXGI_SWAP_CHAIN_DESC scd = {};
    scd.BufferCount = 2;
    scd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    scd.OutputWindow = hwnd;
    scd.SampleDesc.Count = 1;
    scd.Windowed = TRUE;
    scd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

    // 3) Create Device / Swap Chain
    UINT deviceFlags = 0;
#if _DEBUG
    deviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif
    D3D_FEATURE_LEVEL featureLevelsRequested[] = {
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_1,
        D3D_FEATURE_LEVEL_10_0
    };
    D3D_FEATURE_LEVEL featureLevelCreated;

    HRESULT hr = D3D11CreateDeviceAndSwapChain(
        nullptr,
        D3D_DRIVER_TYPE_HARDWARE,
        nullptr,
        deviceFlags,
        featureLevelsRequested,
        _countof(featureLevelsRequested),
        D3D11_SDK_VERSION,
        &scd,
        &out.swapChain, // Swap Chain result
        &out.device, // D3D11 Device result
        &featureLevelCreated,
        &out.context // D3D11 Context result
    );
    if (FAILED(hr))
        throw std::runtime_error("D3D11CreateDeviceAndSwapChain failed");

    // 4) Create Render Target View from Back Buffer
    ID3D11Texture2D* backBuffer = nullptr;
    hr = out.swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&backBuffer);
    if (FAILED(hr))
        throw std::runtime_error("GetBuffer failed");

    hr = out.device->CreateRenderTargetView(backBuffer, nullptr, &out.rtv);
    SafeRelease(backBuffer);
    if (FAILED(hr))
        throw std::runtime_error("CreateRenderTargetView failed");
}

// Destroy D3D11
static void DestroyD3D11(D3DContext& d3d)
{
    SafeRelease(d3d.rtv);
    SafeRelease(d3d.swapChain);
    SafeRelease(d3d.context);
    SafeRelease(d3d.device);
}

int main(int, char**)
{
    SDL_SetMainReady();
    // 1) SDL Initialisation
    if (SDL_Init(SDL_INIT_VIDEO) != 0)
    {
        std::cerr << "SDL_Init Error: " << SDL_GetError() << "\n";
        return -1;
    }

    // 2) Create window
    SDL_Window* window = SDL_CreateWindow(
        "Kibako Sandbox",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        1280, 720,
        SDL_WINDOW_SHOWN
    );
    if (!window)
    {
        std::cerr << "SDL_CreateWindow Error: " << SDL_GetError() << "\n";
        SDL_Quit();
        return -1;
    }

    // 3) D3D11 Initialisation
    D3DContext d3d{};
    try
    {
        CreateD3D11(window, d3d);
    }
    catch (const std::exception& e)
    {
        std::cerr << "D3D init failed: " << e.what() << "\n";
        SDL_DestroyWindow(window);
        SDL_Quit();
        return -1;
    }

    // 4) Main loop (events + clear + present)
    bool running = true;
    while (running)
    {
        SDL_Event e;
        while (SDL_PollEvent(&e))
        {
            if (e.type == SDL_QUIT) running = false;
            if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE) running = false;
        }

        // Set context render target
        d3d.context->OMSetRenderTargets(1, &d3d.rtv, nullptr);

        // Clear screen
        const float color[4] = { 0.10f, 0.12f, 0.16f, 1.0f };
        if (d3d.rtv)
        {
            d3d.context->OMSetRenderTargets(1, &d3d.rtv, nullptr);
            d3d.context->ClearRenderTargetView(d3d.rtv, color);
        }

        // Present (swap back/front)
        d3d.swapChain->Present(1, 0);
    }

    // 5) Cleanup
    DestroyD3D11(d3d);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
