// Kibako2DEngine/src/Core/Application.cpp
#define WIN32_LEAN_AND_MEAN
#define SDL_MAIN_HANDLED

#include <windows.h>
#include <d3d11.h>
#include <dxgi.h>
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")

#include <SDL2/SDL.h>
#include <SDL2/SDL_syswm.h>

#include <stdexcept>
#include <iostream>
#include <cstring>

#include "KibakoEngine/Core/Application.h"

namespace KibakoEngine
{
    // Safe release helper for COM objects
    template<typename T>
    inline void SafeRelease(T*& p) { if (p) { p->Release(); p = nullptr; } }

    // Core Direct3D 11 resources
    struct D3DContext
    {
        ID3D11Device* device = nullptr;
        ID3D11DeviceContext* context = nullptr;
        IDXGISwapChain* swapChain = nullptr;
        ID3D11RenderTargetView* rtv = nullptr;
    };

    struct Application::Impl
    {
        SDL_Window* window = nullptr;
        HWND hwnd = nullptr;
        bool running = false;
        int width = 0;
        int height = 0;

        D3DContext d3d{};

        bool CreateWindowSDL(int w, int h, const char* title)
        {
            SDL_SetMainReady();
            if (SDL_Init(SDL_INIT_VIDEO) != 0)
            {
                std::cerr << "SDL_Init Error: " << SDL_GetError() << "\n";
                return false;
            }

            window = SDL_CreateWindow(
                title,
                SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                w, h,
                SDL_WINDOW_SHOWN
            );
            if (!window)
            {
                std::cerr << "SDL_CreateWindow Error: " << SDL_GetError() << "\n";
                SDL_Quit();
                return false;
            }

            // Get HWND handle for Direct3D
            SDL_SysWMinfo wminfo;
            SDL_VERSION(&wminfo.version);
            if (!SDL_GetWindowWMInfo(window, &wminfo))
            {
                std::cerr << "SDL_GetWindowWMInfo failed\n";
                return false;
            }
            hwnd = wminfo.info.win.window;
            width = w; height = h;
            return true;
        }

        bool CreateDeviceAndSwapchain()
        {
            DXGI_SWAP_CHAIN_DESC scd;
            std::memset(&scd, 0, sizeof(scd));
            scd.BufferCount = 2;
            scd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
            scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
            scd.OutputWindow = hwnd;
            scd.SampleDesc.Count = 1;
            scd.Windowed = TRUE;
            scd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

            UINT deviceFlags = 0;
#if _DEBUG
            deviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

            D3D_FEATURE_LEVEL requested[] = {
                D3D_FEATURE_LEVEL_11_0,
                D3D_FEATURE_LEVEL_10_1,
                D3D_FEATURE_LEVEL_10_0
            };
            D3D_FEATURE_LEVEL created{};

            HRESULT hr = D3D11CreateDeviceAndSwapChain(
                nullptr,
                D3D_DRIVER_TYPE_HARDWARE,
                nullptr, deviceFlags,
                requested, _countof(requested),
                D3D11_SDK_VERSION,
                &scd,
                &d3d.swapChain,
                &d3d.device,
                &created,
                &d3d.context
            );
            if (FAILED(hr))
            {
                std::cerr << "D3D11CreateDeviceAndSwapChain failed (hr=" << std::hex << hr << ")\n";
                return false;
            }
            return CreateRTV();
        }

        bool CreateRTV()
        {
            ID3D11Texture2D* backBuffer = nullptr;
            HRESULT hr = d3d.swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&backBuffer);
            if (FAILED(hr))
            {
                std::cerr << "SwapChain::GetBuffer failed (hr=" << std::hex << hr << ")\n";
                return false;
            }

            hr = d3d.device->CreateRenderTargetView(backBuffer, nullptr, &d3d.rtv);
            SafeRelease(backBuffer);
            if (FAILED(hr))
            {
                std::cerr << "CreateRenderTargetView failed (hr=" << std::hex << hr << ")\n";
                return false;
            }
            return true;
        }

        void DestroyRTV() { SafeRelease(d3d.rtv); }
        void DestroyD3D()
        {
            DestroyRTV();
            SafeRelease(d3d.swapChain);
            SafeRelease(d3d.context);
            SafeRelease(d3d.device);
        }
        void DestroyWindowSDL()
        {
            if (window) { SDL_DestroyWindow(window); window = nullptr; }
            SDL_Quit();
        }

        // Called at the beginning of each frame
        void BeginFrame()
        {
            if (d3d.rtv)
            {
                d3d.context->OMSetRenderTargets(1, &d3d.rtv, nullptr);
                const float color[4] = { 0.10f, 0.12f, 0.16f, 1.0f };
                d3d.context->ClearRenderTargetView(d3d.rtv, color);
            }
        }

        // Called at the end of each frame
        void EndFrame()
        {
            if (d3d.swapChain)
                d3d.swapChain->Present(1, 0); // VSync
        }
    };

    // -------------------------------------------------
    // Public Application methods
    // -------------------------------------------------

    bool Application::Init(int width, int height, const char* title)
    {
        if (m_impl) return true;

        m_impl = new Impl();
        if (!m_impl->CreateWindowSDL(width, height, title))
            return false;

        if (!m_impl->CreateDeviceAndSwapchain())
            return false;

        m_impl->running = true;
        return true;
    }

    void Application::Run()
    {
        if (!m_impl || !m_impl->running) return;

        while (m_impl->running)
        {
            SDL_Event e;
            while (SDL_PollEvent(&e))
            {
                if (e.type == SDL_QUIT) m_impl->running = false;
                if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE)
                    m_impl->running = false;
            }

            m_impl->BeginFrame();
            // TODO: Render scene here
            m_impl->EndFrame();
        }
    }

    void Application::Shutdown()
    {
        if (!m_impl) return;

        m_impl->DestroyD3D();
        m_impl->DestroyWindowSDL();

        delete m_impl;
        m_impl = nullptr;
    }
}
