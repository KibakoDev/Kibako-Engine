// Kibako2DEngine/src/Renderer/RendererD3D11.cpp
#define WIN32_LEAN_AND_MEAN
#include "KibakoEngine/Renderer/RendererD3D11.h"
#include <iostream>
#include <cstring>

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")

namespace KibakoEngine
{
    template<typename T>
    inline void SafeRelease(T*& p) { if (p) { p->Release(); p = nullptr; } }

    bool RendererD3D11::Init(HWND hwnd, int width, int height)
    {
        m_hwnd = hwnd;
        m_width = width;
        m_height = height;

        DXGI_SWAP_CHAIN_DESC scd{};
        scd.BufferCount = 2;
        scd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        scd.OutputWindow = hwnd;
        scd.SampleDesc.Count = 1;
        scd.Windowed = TRUE;
        scd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

        UINT flags = 0;
#if _DEBUG
        flags |= D3D11_CREATE_DEVICE_DEBUG;
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
            nullptr,
            flags,
            requested, _countof(requested),
            D3D11_SDK_VERSION,
            &scd,
            &m_swapChain,
            &m_device,
            &created,
            &m_context
        );

        if (FAILED(hr))
        {
            std::cerr << "Failed to create D3D11 device/swapchain (hr=" << std::hex << hr << ")\n";
            return false;
        }

        if (!CreateRTV())
            return false;

        // Setup initial viewport
        D3D11_VIEWPORT vp{};
        vp.Width = static_cast<float>(width);
        vp.Height = static_cast<float>(height);
        vp.MinDepth = 0.0f;
        vp.MaxDepth = 1.0f;
        vp.TopLeftX = 0;
        vp.TopLeftY = 0;
        m_context->RSSetViewports(1, &vp);

        return true;
    }

    bool RendererD3D11::CreateRTV()
    {
        ID3D11Texture2D* backBuffer = nullptr;
        HRESULT hr = m_swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&backBuffer);
        if (FAILED(hr))
        {
            std::cerr << "SwapChain::GetBuffer failed (hr=" << std::hex << hr << ")\n";
            return false;
        }

        hr = m_device->CreateRenderTargetView(backBuffer, nullptr, &m_rtv);
        SafeRelease(backBuffer);
        if (FAILED(hr))
        {
            std::cerr << "CreateRenderTargetView failed (hr=" << std::hex << hr << ")\n";
            return false;
        }

        return true;
    }

    void RendererD3D11::DestroyRTV()
    {
        SafeRelease(m_rtv);
    }

    void RendererD3D11::BeginFrame()
    {
        if (m_rtv)
        {
            m_context->OMSetRenderTargets(1, &m_rtv, nullptr);
            const float color[4] = { 0.10f, 0.12f, 0.16f, 1.0f };
            m_context->ClearRenderTargetView(m_rtv, color);
        }
    }

    void RendererD3D11::EndFrame()
    {
        if (m_swapChain)
            m_swapChain->Present(1, 0);
    }

    void RendererD3D11::OnResize(int newWidth, int newHeight)
    {
        if (newWidth == 0 || newHeight == 0) return;

        m_width = newWidth;
        m_height = newHeight;

        DestroyRTV();

        HRESULT hr = m_swapChain->ResizeBuffers(0, newWidth, newHeight, DXGI_FORMAT_UNKNOWN, 0);
        if (FAILED(hr))
        {
            std::cerr << "ResizeBuffers failed (hr=" << std::hex << hr << ")\n";
            return;
        }

        if (!CreateRTV()) return;

        D3D11_VIEWPORT vp{};
        vp.Width = static_cast<float>(newWidth);
        vp.Height = static_cast<float>(newHeight);
        vp.MinDepth = 0.0f;
        vp.MaxDepth = 1.0f;
        vp.TopLeftX = 0;
        vp.TopLeftY = 0;
        m_context->RSSetViewports(1, &vp);
    }

    void RendererD3D11::Shutdown()
    {
        DestroyRTV();
        SafeRelease(m_swapChain);
        SafeRelease(m_context);
        SafeRelease(m_device);
    }
}