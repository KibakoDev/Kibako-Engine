// Kibako2DEngine/src/Renderer/RendererD3D11.cpp
#define WIN32_LEAN_AND_MEAN
#include "KibakoEngine/Renderer/RendererD3D11.h"

#include <iostream>
#include <cstring>

#include "KibakoEngine/Core/Debug.h"
#include "KibakoEngine/Core/Log.h"

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")

using Microsoft::WRL::ComPtr;

namespace KibakoEngine {

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
        KBK_HR(hr);
        if (FAILED(hr)) {
            std::cerr << "D3D11CreateDeviceAndSwapChain failed (hr=0x" << std::hex << hr << ")\n";
            return false;
        }

        if (!CreateRTV()) return false;

        D3D11_VIEWPORT vp{};
        vp.Width = static_cast<float>(width);
        vp.Height = static_cast<float>(height);
        vp.MinDepth = 0.0f;
        vp.MaxDepth = 1.0f;
        vp.TopLeftX = 0;
        vp.TopLeftY = 0;
        m_context->RSSetViewports(1, &vp);

        // Camera config
        m_camera.SetVirtualSize(width, height);
        m_camera.SetViewportSize(width, height);
        m_camera.SetPosition(0.0f, 0.0f);
        m_camera.SetRotation(0.0f);

        if (!CreateConstantBuffers()) return false;

        // --- Init SpriteBatch2D ---
        if (!m_batch.Init(m_device.Get(), m_context.Get())) {
            std::cerr << "SpriteBatch2D::Init failed\n";
            return false;
        }
        m_batch.SetPointSampling(true);
        m_batch.SetPixelSnap(true);
        m_batch.SetMonochrome(0.0f);

        KbkLog("Renderer", "RendererD3D11 initialized (%dx%d)", m_width, m_height);
        return true;
    }

    bool RendererD3D11::CreateRTV()
    {
        ComPtr<ID3D11Texture2D> backBuffer;
        HRESULT hr = m_swapChain->GetBuffer(
            0, __uuidof(ID3D11Texture2D),
            reinterpret_cast<void**>(backBuffer.GetAddressOf()));
        KBK_HR(hr);
        if (FAILED(hr)) return false;

        hr = m_device->CreateRenderTargetView(backBuffer.Get(), nullptr, m_rtv.ReleaseAndGetAddressOf());
        KBK_HR(hr);
        if (FAILED(hr)) return false;

        return true;
    }

    void RendererD3D11::DestroyRTV()
    {
        m_rtv.Reset();
    }

    bool RendererD3D11::CreateConstantBuffers()
    {
        D3D11_BUFFER_DESC bd{};
        bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
        bd.ByteWidth = sizeof(CB_VS_Camera);
        bd.Usage = D3D11_USAGE_DYNAMIC;
        bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

        HRESULT hr = m_device->CreateBuffer(&bd, nullptr, m_cbCamera.ReleaseAndGetAddressOf());
        KBK_HR(hr);
        return SUCCEEDED(hr);
    }

    void RendererD3D11::UpdateCameraCB()
    {
        CB_VS_Camera data{};
        data.ViewProj = m_camera.GetViewProjT();

        D3D11_MAPPED_SUBRESOURCE map{};
        HRESULT hr = m_context->Map(m_cbCamera.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &map);
        KBK_HR(hr);
        if (FAILED(hr)) return;

        std::memcpy(map.pData, &data, sizeof(data));
        m_context->Unmap(m_cbCamera.Get(), 0);

        ID3D11Buffer* cb = m_cbCamera.Get();
        m_context->VSSetConstantBuffers(0, 1, &cb);
    }

    void RendererD3D11::BeginFrame()
    {
        if (!m_rtv) return;

        ID3D11RenderTargetView* rtv = m_rtv.Get();
        m_context->OMSetRenderTargets(1, &rtv, nullptr);

        const float color[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
        m_context->ClearRenderTargetView(m_rtv.Get(), color);

        UpdateCameraCB();
    }

    void RendererD3D11::EndFrame()
    {
        if (m_swapChain) m_swapChain->Present(1, 0);
    }

    void RendererD3D11::OnResize(int newWidth, int newHeight)
    {
        if (newWidth <= 0 || newHeight <= 0) return;

        m_width = newWidth;
        m_height = newHeight;

        ID3D11RenderTargetView* nullRTV[] = { nullptr };
        m_context->OMSetRenderTargets(1, nullRTV, nullptr);

        DestroyRTV();

        HRESULT hr = m_swapChain->ResizeBuffers(0, m_width, m_height, DXGI_FORMAT_UNKNOWN, 0);
        KBK_HR(hr);
        if (FAILED(hr)) return;

        if (!CreateRTV()) return;

        D3D11_VIEWPORT vp{};
        vp.Width = static_cast<float>(m_width);
        vp.Height = static_cast<float>(m_height);
        vp.MinDepth = 0.0f;
        vp.MaxDepth = 1.0f;
        vp.TopLeftX = 0;
        vp.TopLeftY = 0;
        m_context->RSSetViewports(1, &vp);

        m_camera.SetViewportSize(newWidth, newHeight);
        m_camera.SetVirtualSize(newWidth, newHeight);

        KbkLog("Renderer", "Resized to %dx%d", m_width, m_height);
    }

    void RendererD3D11::Shutdown()
    {
        KbkLog("Renderer", "Shutdown");
        m_batch.Shutdown();
        DestroyRTV();
        m_swapChain.Reset();
        m_context.Reset();
        m_device.Reset();
    }

}