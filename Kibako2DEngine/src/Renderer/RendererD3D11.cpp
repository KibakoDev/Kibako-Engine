// Kibako2DEngine/src/Renderer/RendererD3D11.cpp
#define WIN32_LEAN_AND_MEAN
#include "KibakoEngine/Renderer/RendererD3D11.h"

#include <iostream>
#include <cstring>

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")

using Microsoft::WRL::ComPtr;

namespace KibakoEngine {

    // =====================================================
    // RENDERER INITIALIZATION
    // =====================================================
    bool RendererD3D11::Init(HWND hwnd, int width, int height)
    {
        m_hwnd = hwnd;
        m_width = width;
        m_height = height;

        // Create device and swap chain (flip model for smooth resize)
        DXGI_SWAP_CHAIN_DESC scd{};
        scd.BufferCount = 2;
        scd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        scd.OutputWindow = hwnd;
        scd.SampleDesc.Count = 1;
        scd.Windowed = TRUE;
        scd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;

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
            nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, flags,
            requested, _countof(requested), D3D11_SDK_VERSION,
            &scd,
            m_swapChain.ReleaseAndGetAddressOf(),
            m_device.ReleaseAndGetAddressOf(),
            &created,
            m_context.ReleaseAndGetAddressOf()
        );
        if (FAILED(hr))
        {
            std::cerr << "D3D11CreateDeviceAndSwapChain failed (hr=0x" << std::hex << hr << ")\n";
            return false;
        }

        // Back buffer -> RTV
        if (!CreateRTV())
            return false;

        // Initial viewport
        D3D11_VIEWPORT vp{};
        vp.Width = static_cast<float>(width);
        vp.Height = static_cast<float>(height);
        vp.MinDepth = 0.0f;
        vp.MaxDepth = 1.0f;
        vp.TopLeftX = 0;
        vp.TopLeftY = 0;
        m_context->RSSetViewports(1, &vp);

        // Camera setup (virtual space == window size to start)
        m_camera.SetVirtualSize(width, height);
        m_camera.SetViewportSize(width, height);
        m_camera.SetPosition(0.0f, 0.0f);
        m_camera.SetZoom(1.0f);
        m_camera.SetRotation(0.0f);

        // Constant buffers (camera)
        if (!CreateConstantBuffers())
            return false;

        // Init the sprite renderer (App will call Begin/End each frame)
        if (!m_spriteRenderer.Init(m_device.Get(), m_context.Get()))
        {
            std::cerr << "SpriteRenderer2D::Init failed\n";
            return false;
        }
        m_spriteRenderer.SetMonochrome(0.0f);

        return true;
    }

    // =====================================================
    // RTV LIFECYCLE
    // =====================================================
    bool RendererD3D11::CreateRTV()
    {
        ComPtr<ID3D11Texture2D> backBuffer;
        HRESULT hr = m_swapChain->GetBuffer(
            0, __uuidof(ID3D11Texture2D),
            reinterpret_cast<void**>(backBuffer.GetAddressOf())
        );
        if (FAILED(hr))
        {
            std::cerr << "SwapChain::GetBuffer failed (hr=0x" << std::hex << hr << ")\n";
            return false;
        }

        hr = m_device->CreateRenderTargetView(backBuffer.Get(), nullptr, m_rtv.ReleaseAndGetAddressOf());
        if (FAILED(hr))
        {
            std::cerr << "CreateRenderTargetView failed (hr=0x" << std::hex << hr << ")\n";
            return false;
        }
        return true;
    }

    void RendererD3D11::DestroyRTV()
    {
        m_rtv.Reset();
    }

    // =====================================================
    // CONSTANT BUFFERS (CAMERA)
    // =====================================================
    bool RendererD3D11::CreateConstantBuffers()
    {
        D3D11_BUFFER_DESC bd{};
        bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
        bd.ByteWidth = sizeof(CB_VS_Camera);
        bd.Usage = D3D11_USAGE_DYNAMIC;
        bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

        HRESULT hr = m_device->CreateBuffer(&bd, nullptr, m_cbCamera.ReleaseAndGetAddressOf());
        if (FAILED(hr))
        {
            std::cerr << "CreateBuffer(CB_VS_Camera) failed (hr=0x" << std::hex << hr << ")\n";
            return false;
        }
        return true;
    }

    void RendererD3D11::UpdateCameraCB()
    {
        CB_VS_Camera data{};
        data.ViewProj = m_camera.GetViewProjT(); // transposed for HLSL row-major

        D3D11_MAPPED_SUBRESOURCE map{};
        if (SUCCEEDED(m_context->Map(m_cbCamera.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &map)))
        {
            std::memcpy(map.pData, &data, sizeof(data));
            m_context->Unmap(m_cbCamera.Get(), 0);
        }

        ID3D11Buffer* cb = m_cbCamera.Get();
        m_context->VSSetConstantBuffers(0, 1, &cb);
    }

    // =====================================================
    // PER-FRAME
    // =====================================================
    void RendererD3D11::BeginFrame()
    {
        if (!m_rtv) return;

        // Bind RT and clear
        ID3D11RenderTargetView* rtv = m_rtv.Get();
        m_context->OMSetRenderTargets(1, &rtv, nullptr);

        const float color[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
        m_context->ClearRenderTargetView(m_rtv.Get(), color);

        // Update camera constants
        UpdateCameraCB();

        // Note: SpriteRenderer Begin/End is called from Application.cpp (not here)
    }

    void RendererD3D11::EndFrame()
    {
        if (m_swapChain)
            m_swapChain->Present(1, 0); // vsync on
    }

    // =====================================================
    // RESIZE
    // =====================================================
    void RendererD3D11::OnResize(int newWidth, int newHeight)
    {
        if (newWidth <= 0 || newHeight <= 0) return;

        m_width = newWidth;
        m_height = newHeight;

        // Unbind RT before resizing
        ID3D11RenderTargetView* nullRTV[] = { nullptr };
        m_context->OMSetRenderTargets(1, nullRTV, nullptr);

        DestroyRTV();

        HRESULT hr = m_swapChain->ResizeBuffers(0, m_width, m_height, DXGI_FORMAT_UNKNOWN, 0);
        if (FAILED(hr))
        {
            std::cerr << "ResizeBuffers failed (hr=0x" << std::hex << hr << ")\n";
            return;
        }

        if (!CreateRTV())
            return;

        // Update viewport
        D3D11_VIEWPORT vp{};
        vp.Width = (float)m_width;
        vp.Height = (float)m_height;
        vp.MinDepth = 0.0f;
        vp.MaxDepth = 1.0f;
        vp.TopLeftX = 0;
        vp.TopLeftY = 0;
        m_context->RSSetViewports(1, &vp);

        // Keep camera in sync
        m_camera.SetViewportSize(newWidth, newHeight);
        m_camera.SetVirtualSize(newWidth, newHeight);
    }

    // =====================================================
    // SHUTDOWN
    // =====================================================
    void RendererD3D11::Shutdown()
    {
        m_spriteRenderer.Shutdown();
        DestroyRTV();
        m_swapChain.Reset();
        m_context.Reset();
        m_device.Reset();
    }

}