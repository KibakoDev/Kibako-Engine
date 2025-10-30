// Kibako2DEngine/src/Renderer/RendererD3D11.cpp
#define WIN32_LEAN_AND_MEAN
#include "KibakoEngine/Renderer/RendererD3D11.h"
#include <iostream>
#include <cstring>

#include <d3dcompiler.h>  // compile HLSL at runtime
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")

namespace KibakoEngine {

    // =====================================================
    // RENDERER INITIALIZATION
    // =====================================================
    bool RendererD3D11::Init(HWND hwnd, int width, int height)
    {
        m_hwnd = hwnd;
        m_width = width;
        m_height = height;

        // Create device and swapchain (flip model for smooth resize)
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
            std::cerr << "Failed to create D3D11 device/swapchain (hr=0x" << std::hex << hr << ")\n";
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

        // Minimal pipeline (shaders + vertex buffer) to draw a triangle
        if (!InitTrianglePipeline())
            return false;

        return true;
    }

    // =====================================================
    // RTV LIFECYCLE
    // =====================================================
    bool RendererD3D11::CreateRTV()
    {
        // Get back buffer from swapchain
        ComPtr<ID3D11Texture2D> backBuffer;
        HRESULT hr = m_swapChain->GetBuffer(
            0, __uuidof(ID3D11Texture2D),
            reinterpret_cast<void**>(backBuffer.GetAddressOf()));
        if (FAILED(hr))
        {
            std::cerr << "SwapChain::GetBuffer failed (hr=0x" << std::hex << hr << ")\n";
            return false;
        }

        // Create render target view on the back buffer
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
    // TRIANGLE PIPELINE (shaders + vertex buffer)
    // =====================================================

    // Simple inline HLSL shaders
    static const char* g_VS_HLSL = R"(
struct VSIn  { float3 pos : POSITION; float3 color : COLOR; };
struct VSOut { float4 pos : SV_Position; float3 color : COLOR; };
VSOut mainVS(VSIn i) {
    VSOut o;
    o.pos = float4(i.pos, 1.0);
    o.color = i.color;
    return o;
}
)";

    static const char* g_PS_HLSL = R"(
struct PSIn { float4 pos : SV_Position; float3 color : COLOR; };
float4 mainPS(PSIn i) : SV_Target {
    return float4(i.color, 1.0);
}
)";

    bool RendererD3D11::InitTrianglePipeline()
    {
        // Compile shaders (in-memory)
        Microsoft::WRL::ComPtr<ID3DBlob> vsBlob, psBlob, err;

        HRESULT hr = D3DCompile(
            g_VS_HLSL, strlen(g_VS_HLSL),
            nullptr, nullptr, nullptr,
            "mainVS", "vs_5_0",
            0, 0,
            vsBlob.GetAddressOf(),
            err.GetAddressOf()
        );
        if (FAILED(hr))
        {
            if (err) { std::cerr << "VS compile error: " << (const char*)err->GetBufferPointer() << "\n"; }
            return false;
        }

        err.Reset();
        hr = D3DCompile(
            g_PS_HLSL, strlen(g_PS_HLSL),
            nullptr, nullptr, nullptr,
            "mainPS", "ps_5_0",
            0, 0,
            psBlob.GetAddressOf(),
            err.GetAddressOf()
        );
        if (FAILED(hr))
        {
            if (err) { std::cerr << "PS compile error: " << (const char*)err->GetBufferPointer() << "\n"; }
            return false;
        }

        // Create shader objects
        hr = m_device->CreateVertexShader(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), nullptr, m_vs.ReleaseAndGetAddressOf());
        if (FAILED(hr)) { std::cerr << "CreateVertexShader failed (hr=0x" << std::hex << hr << ")\n"; return false; }

        hr = m_device->CreatePixelShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(), nullptr, m_ps.ReleaseAndGetAddressOf());
        if (FAILED(hr)) { std::cerr << "CreatePixelShader failed (hr=0x" << std::hex << hr << ")\n"; return false; }

        // Describe vertex memory layout (POSITION float3, COLOR float3)
        D3D11_INPUT_ELEMENT_DESC layout[] = {
            { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,                 D3D11_INPUT_PER_VERTEX_DATA, 0 },
            { "COLOR",    0, DXGI_FORMAT_R32G32B32_FLOAT, 0, sizeof(float) * 3, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        };
        hr = m_device->CreateInputLayout(
            layout, _countof(layout),
            vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(),
            m_inputLayout.ReleaseAndGetAddressOf()
        );
        if (FAILED(hr)) { std::cerr << "CreateInputLayout failed (hr=0x" << std::hex << hr << ")\n"; return false; }

        // Create a small vertex buffer in clip space
        struct Vertex { float pos[3]; float color[3]; };
        const Vertex verts[3] = {
            { {  0.0f,  0.5f, 0.0f }, {  1.0f, 1.0f, 1.0f } },
            { {  0.5f, -0.5f, 0.0f }, {  1.0f, 1.0f, 1.0f } },
            { { -0.5f, -0.5f, 0.0f }, {  1.0f, 1.0f, 1.0f } },
        };

        D3D11_BUFFER_DESC bd{};
        bd.ByteWidth = sizeof(verts);
        bd.Usage = D3D11_USAGE_IMMUTABLE; // static data
        bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;

        D3D11_SUBRESOURCE_DATA srd{};
        srd.pSysMem = verts;

        hr = m_device->CreateBuffer(&bd, &srd, m_vb.ReleaseAndGetAddressOf());
        if (FAILED(hr))
        {
            std::cerr << "CreateBuffer failed (hr=0x" << std::hex << hr << ")\n";
            return false;
        }

        m_vbStride = sizeof(Vertex);
        m_vbOffset = 0;

        // Triangle list: each 3 vertices form one triangle
        m_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        return true;
    }

    void RendererD3D11::DrawTriangle()
    {
        // Bind pipeline state and vertex buffer, then draw
        m_context->IASetInputLayout(m_inputLayout.Get());
        m_context->VSSetShader(m_vs.Get(), nullptr, 0);
        m_context->PSSetShader(m_ps.Get(), nullptr, 0);

        ID3D11Buffer* vb = m_vb.Get();
        UINT stride = m_vbStride;
        UINT offset = m_vbOffset;
        m_context->IASetVertexBuffers(0, 1, &vb, &stride, &offset);

        m_context->Draw(3, 0);
    }

    void RendererD3D11::DestroyTriangle()
    {
        m_vb.Reset();
        m_inputLayout.Reset();
        m_vs.Reset();
        m_ps.Reset();
    }

    // =====================================================
    // PER-FRAME RENDER
    // =====================================================
    void RendererD3D11::BeginFrame()
    {
        if (m_rtv)
        {
            // Bind render target and clear screen
            ID3D11RenderTargetView* rtv = m_rtv.Get();
            m_context->OMSetRenderTargets(1, &rtv, nullptr);

            const float color[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
            m_context->ClearRenderTargetView(m_rtv.Get(), color);

            // Draw one triangle
            DrawTriangle();
        }
    }

    void RendererD3D11::EndFrame()
    {
        if (m_swapChain)
            m_swapChain->Present(1, 0); // vsync on
    }

    // =====================================================
    // RESIZE HANDLING
    // =====================================================
    void RendererD3D11::OnResize(int newWidth, int newHeight)
    {
        if (newWidth <= 0 || newHeight <= 0) return;
        m_width = newWidth;
        m_height = newHeight;

        // Unbind current RTV before resizing
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
    }

    // =====================================================
    // SHUTDOWN
    // =====================================================
    void RendererD3D11::Shutdown()
    {
        DestroyTriangle();
        DestroyRTV();
        m_swapChain.Reset();
        m_context.Reset();
        m_device.Reset();
    }

}