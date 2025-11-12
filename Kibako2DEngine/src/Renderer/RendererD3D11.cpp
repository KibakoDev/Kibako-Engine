#define WIN32_LEAN_AND_MEAN
#define NOMINMAX

#include "KibakoEngine/Renderer/RendererD3D11.h"

#include "KibakoEngine/Core/Debug.h"
#include "KibakoEngine/Core/Log.h"

#include <iterator>
#include <windows.h>

namespace KibakoEngine {

    bool RendererD3D11::Init(HWND hwnd, uint32_t width, uint32_t height)
    {
        m_width = width;
        m_height = height;

        if (!CreateSwapChain(hwnd, width, height))
            return false;
        if (!CreateRenderTargets(width, height))
            return false;
        if (!m_batch.Init(m_device.Get(), m_context.Get()))
            return false;

        m_camera.SetPosition(0.0f, 0.0f);
        m_camera.SetRotation(0.0f);
        return true;
    }

    void RendererD3D11::Shutdown()
    {
        m_batch.Shutdown();
        if (m_context)
            m_context->ClearState();
        m_rtv.Reset();
        m_swapChain.Reset();
        m_context.Reset();
        m_device.Reset();
        m_width = 0;
        m_height = 0;
    }

    void RendererD3D11::BeginFrame(const float clearColor[4])
    {
        const float color[4] = {
            clearColor ? clearColor[0] : 0.0f,
            clearColor ? clearColor[1] : 0.0f,
            clearColor ? clearColor[2] : 0.0f,
            clearColor ? clearColor[3] : 1.0f,
        };

        ID3D11RenderTargetView* rtv = m_rtv.Get();
        m_context->OMSetRenderTargets(1, &rtv, nullptr);

        D3D11_VIEWPORT vp{};
        vp.TopLeftX = 0.0f;
        vp.TopLeftY = 0.0f;
        vp.Width = static_cast<float>(m_width);
        vp.Height = static_cast<float>(m_height);
        vp.MinDepth = 0.0f;
        vp.MaxDepth = 1.0f;
        m_context->RSSetViewports(1, &vp);

        m_context->ClearRenderTargetView(m_rtv.Get(), color);
    }

    void RendererD3D11::EndFrame(bool waitForVSync)
    {
        if (m_swapChain)
            m_swapChain->Present(waitForVSync ? 1 : 0, 0);
    }

    void RendererD3D11::OnResize(uint32_t width, uint32_t height)
    {
        if (!m_swapChain)
            return;
        if (width == 0 || height == 0)
            return;
        if (width == m_width && height == m_height)
            return;

        m_width = width;
        m_height = height;

        m_context->OMSetRenderTargets(0, nullptr, nullptr);
        m_rtv.Reset();
        HRESULT hr = m_swapChain->ResizeBuffers(0, width, height, DXGI_FORMAT_UNKNOWN, 0);
        KBK_HR(hr);
        if (FAILED(hr))
            return;

        if (!CreateRenderTargets(width, height))
            return;

        m_camera.SetViewport(static_cast<float>(width), static_cast<float>(height));
    }

    bool RendererD3D11::CreateSwapChain(HWND hwnd, uint32_t width, uint32_t height)
    {
        UINT flags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
    #if defined(_DEBUG)
        flags |= D3D11_CREATE_DEVICE_DEBUG;
    #endif

        DXGI_SWAP_CHAIN_DESC desc{};
        desc.BufferCount = 2;
        desc.BufferDesc.Width = width;
        desc.BufferDesc.Height = height;
        desc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        desc.OutputWindow = hwnd;
        desc.SampleDesc.Count = 1;
        desc.Windowed = TRUE;
        desc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
        desc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

        D3D_FEATURE_LEVEL requestedLevels[] = {
            D3D_FEATURE_LEVEL_11_1,
            D3D_FEATURE_LEVEL_11_0,
            D3D_FEATURE_LEVEL_10_1,
            D3D_FEATURE_LEVEL_10_0,
        };

        HRESULT hr = D3D11CreateDeviceAndSwapChain(nullptr,
                                                   D3D_DRIVER_TYPE_HARDWARE,
                                                   nullptr,
                                                   flags,
                                                   requestedLevels,
                                                   static_cast<UINT>(std::size(requestedLevels)),
                                                   D3D11_SDK_VERSION,
                                                   &desc,
                                                   m_swapChain.GetAddressOf(),
                                                   m_device.GetAddressOf(),
                                                   &m_featureLevel,
                                                   m_context.GetAddressOf());
    #if defined(_DEBUG)
        if (FAILED(hr)) {
            flags &= ~D3D11_CREATE_DEVICE_DEBUG;
            hr = D3D11CreateDeviceAndSwapChain(nullptr,
                                               D3D_DRIVER_TYPE_HARDWARE,
                                               nullptr,
                                               flags,
                                               requestedLevels,
                                               static_cast<UINT>(std::size(requestedLevels)),
                                               D3D11_SDK_VERSION,
                                               &desc,
                                               m_swapChain.GetAddressOf(),
                                               m_device.GetAddressOf(),
                                               &m_featureLevel,
                                               m_context.GetAddressOf());
        }
    #endif
        KBK_HR(hr);
        if (FAILED(hr))
            return false;

        KbkLog("Renderer", "D3D11 feature level: 0x%04X", static_cast<unsigned>(m_featureLevel));
        return true;
    }

    bool RendererD3D11::CreateRenderTargets(uint32_t width, uint32_t height)
    {
        Microsoft::WRL::ComPtr<ID3D11Texture2D> backBuffer;
        HRESULT hr = m_swapChain->GetBuffer(0, IID_PPV_ARGS(backBuffer.GetAddressOf()));
        KBK_HR(hr);
        if (FAILED(hr))
            return false;

        hr = m_device->CreateRenderTargetView(backBuffer.Get(), nullptr, m_rtv.GetAddressOf());
        KBK_HR(hr);
        if (FAILED(hr))
            return false;

        m_width = width;
        m_height = height;
        m_camera.SetViewport(static_cast<float>(width), static_cast<float>(height));
        return true;
    }

} // namespace KibakoEngine

