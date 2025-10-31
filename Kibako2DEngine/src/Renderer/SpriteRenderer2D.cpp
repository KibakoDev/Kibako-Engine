// =====================================================
// Kibako2DEngine/Renderer/SpriteRenderer2D.cpp
// Immediate-mode 2D sprite renderer for D3D11.
// - Pixel-perfect sampling (point filtering)
// - Alpha blending (SrcAlpha, InvSrcAlpha)
// - Optional grayscale tint via constant buffer
// =====================================================

#define WIN32_LEAN_AND_MEAN
#include "KibakoEngine/Renderer/SpriteRenderer2D.h"
#include <d3dcompiler.h>
#include <cmath>
#include <iostream>

#pragma comment(lib, "d3dcompiler.lib")

using namespace DirectX;

namespace KibakoEngine {

    // =====================================================
    // HLSL source (embedded)
    // =====================================================

    // Vertex shader: transforms to clip space and forwards UV/color
    static const char* kVS = R"(
cbuffer CB_VS_Transform : register(b0)
{
    float4x4 gViewProj;
};
struct VSIn {
    float3 pos : POSITION;
    float2 uv  : TEXCOORD;
    float4 col : COLOR;
};
struct VSOut {
    float4 pos : SV_Position;
    float2 uv  : TEXCOORD;
    float4 col : COLOR;
};
VSOut mainVS(VSIn i) {
    VSOut o;
    o.pos = mul(float4(i.pos, 1.0), gViewProj);
    o.uv  = i.uv;
    o.col = i.col;
    return o;
})";

    // Pixel shader: samples the texture, optional grayscale, applies vertex tint
    static const char* kPS = R"(
Texture2D tex0 : register(t0);
SamplerState samp0 : register(s0);

cbuffer CB_PS_Params : register(b0)
{
    float Monochrome;   // 0..1
    float3 pad;
};

float3 Luma(float3 rgb) {
    // Rec.601 luma coefficients
    float y = dot(rgb, float3(0.299, 0.587, 0.114));
    return float3(y, y, y);
}

float4 mainPS(float4 pos : SV_Position, float2 uv : TEXCOORD, float4 col : COLOR) : SV_Target
{
    float4 t = tex0.Sample(samp0, uv);
    float3 rgb = lerp(t.rgb, Luma(t.rgb), Monochrome);
    return float4(rgb * col.rgb, t.a * col.a);
})";

    // =====================================================
    // Public API
    // =====================================================

    bool SpriteRenderer2D::Init(ID3D11Device* device, ID3D11DeviceContext* context)
    {
        m_device = device;
        m_context = context;

        if (!CreateShaders(device))    return false;
        if (!CreateBuffers(device))    return false;
        if (!CreateStates(device))     return false;

        return true;
    }

    void SpriteRenderer2D::Shutdown()
    {
        m_blend.Reset();
        m_sampler.Reset();
        m_cbPS.Reset();
        m_cbVS.Reset();
        m_vb.Reset();
        m_inputLayout.Reset();
        m_ps.Reset();
        m_vs.Reset();
        m_device = nullptr;
        m_context = nullptr;
        m_isDrawing = false;
        m_monochrome = 0.0f;
        m_viewProj = XMFLOAT4X4{};
    }

    void SpriteRenderer2D::Begin(const XMFLOAT4X4& viewProj)
    {
        if (m_isDrawing) return;
        m_isDrawing = true;
        m_viewProj = viewProj;

        // Update camera CB (VS b0)
        D3D11_MAPPED_SUBRESOURCE map{};
        if (SUCCEEDED(m_context->Map(m_cbVS.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &map)))
        {
            auto* cb = reinterpret_cast<CB_VS_Transform*>(map.pData);
            cb->ViewProj = viewProj;
            m_context->Unmap(m_cbVS.Get(), 0);
        }

        // Bind VS constants
        ID3D11Buffer* cbVS = m_cbVS.Get();
        m_context->VSSetConstantBuffers(0, 1, &cbVS);

        // Set fixed states used by all sprite draws this frame
        // Input Assembler
        UINT stride = sizeof(Vertex);
        UINT offset = 0;
        ID3D11Buffer* vb = m_vb.Get();
        m_context->IASetVertexBuffers(0, 1, &vb, &stride, &offset);
        m_context->IASetInputLayout(m_inputLayout.Get());
        m_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

        // Shaders
        m_context->VSSetShader(m_vs.Get(), nullptr, 0);
        m_context->PSSetShader(m_ps.Get(), nullptr, 0);

        // Sampler (point clamp) in PS s0
        ID3D11SamplerState* samp = m_sampler.Get();
        m_context->PSSetSamplers(0, 1, &samp);

        // Blend state (alpha blend)
        float blendFactor[4] = { 0,0,0,0 };
        m_context->OMSetBlendState(m_blend.Get(), blendFactor, 0xFFFFFFFF);
    }

    void SpriteRenderer2D::End()
    {
        if (!m_isDrawing) return;
        m_isDrawing = false;

        // Optional: unbind to keep D3D debug happy in more complex apps
        ID3D11ShaderResourceView* nullSRV[1] = { nullptr };
        m_context->PSSetShaderResources(0, 1, nullSRV);
    }

    void SpriteRenderer2D::DrawSprite(const Texture2D& tex,
        const RectF& dst,
        const RectF& src,
        const Color4& color,
        float rotation)
    {
        if (!m_isDrawing || !tex.GetSRV()) return;

        // Update pixel shader constants (PS b0)
        D3D11_MAPPED_SUBRESOURCE map{};
        if (SUCCEEDED(m_context->Map(m_cbPS.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &map)))
        {
            auto* cb = reinterpret_cast<CB_PS_Params*>(map.pData);
            cb->Monochrome = m_monochrome;
            m_context->Unmap(m_cbPS.Get(), 0);
        }
        ID3D11Buffer* cbPS = m_cbPS.Get();
        m_context->PSSetConstantBuffers(0, 1, &cbPS);

        // Compute quad corners (rotation about center)
        XMFLOAT2 c = { dst.x + dst.w * 0.5f, dst.y + dst.h * 0.5f };
        XMFLOAT2 p[4] = {
            { dst.x,         dst.y          }, // TL
            { dst.x + dst.w, dst.y          }, // TR
            { dst.x + dst.w, dst.y + dst.h  }, // BR
            { dst.x,         dst.y + dst.h  }  // BL
        };

        if (rotation != 0.0f) {
            float cs = std::cos(rotation);
            float sn = std::sin(rotation);
            for (auto& v : p) {
                float dx = v.x - c.x;
                float dy = v.y - c.y;
                v.x = c.x + dx * cs - dy * sn;
                v.y = c.y + dx * sn + dy * cs;
            }
        }

        // Build 2 triangles (6 vertices)
        float u1 = src.x;
        float v1 = src.y;
        float u2 = src.x + src.w;
        float v2 = src.y + src.h;

        const XMFLOAT4 col = { color.r, color.g, color.b, color.a };

        Vertex verts[6] = {
            { { p[0].x, p[0].y, 0.0f }, { u1, v1 }, col }, // TL
            { { p[1].x, p[1].y, 0.0f }, { u2, v1 }, col }, // TR
            { { p[2].x, p[2].y, 0.0f }, { u2, v2 }, col }, // BR
            { { p[0].x, p[0].y, 0.0f }, { u1, v1 }, col }, // TL
            { { p[2].x, p[2].y, 0.0f }, { u2, v2 }, col }, // BR
            { { p[3].x, p[3].y, 0.0f }, { u1, v2 }, col }, // BL
        };

        // Upload vertices for this sprite
        m_context->UpdateSubresource(m_vb.Get(), 0, nullptr, verts, 0, 0);

        // Draw
        FlushDraw(tex, 6);
    }

    // =====================================================
    // Internal helpers
    // =====================================================

    bool SpriteRenderer2D::CreateShaders(ID3D11Device* device)
    {
        Microsoft::WRL::ComPtr<ID3DBlob> vsBlob, psBlob, err;

        HRESULT hr = D3DCompile(kVS, strlen(kVS),
            nullptr, nullptr, nullptr, "mainVS", "vs_5_0",
            0, 0, vsBlob.GetAddressOf(), err.GetAddressOf());
        if (FAILED(hr)) {
            if (err) std::cerr << "VS compile error: " << (const char*)err->GetBufferPointer() << "\n";
            return false;
        }

        err.Reset();
        hr = D3DCompile(kPS, strlen(kPS),
            nullptr, nullptr, nullptr, "mainPS", "ps_5_0",
            0, 0, psBlob.GetAddressOf(), err.GetAddressOf());
        if (FAILED(hr)) {
            if (err) std::cerr << "PS compile error: " << (const char*)err->GetBufferPointer() << "\n";
            return false;
        }

        // Create shaders
        hr = device->CreateVertexShader(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), nullptr, m_vs.ReleaseAndGetAddressOf());
        if (FAILED(hr)) return false;

        hr = device->CreatePixelShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(), nullptr, m_ps.ReleaseAndGetAddressOf());
        if (FAILED(hr)) return false;

        // Input layout (POSITION float3, TEXCOORD float2, COLOR float4)
        D3D11_INPUT_ELEMENT_DESC layout[] = {
            { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,      0,  0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
            { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,         0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
            { "COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT,   0, 20, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        };

        hr = device->CreateInputLayout(layout, _countof(layout),
            vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(),
            m_inputLayout.ReleaseAndGetAddressOf());
        return SUCCEEDED(hr);
    }

    bool SpriteRenderer2D::CreateBuffers(ID3D11Device* device)
    {
        // Dynamic vertex buffer for 6 vertices per quad
        D3D11_BUFFER_DESC vbDesc{};
        vbDesc.ByteWidth = sizeof(Vertex) * 6;
        vbDesc.Usage = D3D11_USAGE_DEFAULT;
        vbDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

        HRESULT hr = device->CreateBuffer(&vbDesc, nullptr, m_vb.ReleaseAndGetAddressOf());
        if (FAILED(hr)) return false;

        // VS constant buffer (camera)
        D3D11_BUFFER_DESC cbDesc{};
        cbDesc.ByteWidth = sizeof(CB_VS_Transform);
        cbDesc.Usage = D3D11_USAGE_DYNAMIC;
        cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
        cbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

        hr = device->CreateBuffer(&cbDesc, nullptr, m_cbVS.ReleaseAndGetAddressOf());
        if (FAILED(hr)) return false;

        // PS constant buffer (monochrome)
        cbDesc.ByteWidth = sizeof(CB_PS_Params);
        hr = device->CreateBuffer(&cbDesc, nullptr, m_cbPS.ReleaseAndGetAddressOf());
        if (FAILED(hr)) return false;

        return true;
    }

    bool SpriteRenderer2D::CreateStates(ID3D11Device* device)
    {
        // Sampler state: point filtering, clamp
        D3D11_SAMPLER_DESC sd{};
        sd.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
        sd.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
        sd.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
        sd.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
        sd.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
        sd.MinLOD = 0;
        sd.MaxLOD = D3D11_FLOAT32_MAX;

        HRESULT hr = device->CreateSamplerState(&sd, m_sampler.ReleaseAndGetAddressOf());
        if (FAILED(hr)) return false;

        // Alpha blend state: SrcAlpha * Src + (1-SrcAlpha) * Dest
        D3D11_BLEND_DESC bd{};
        bd.AlphaToCoverageEnable = FALSE;
        bd.IndependentBlendEnable = FALSE;

        D3D11_RENDER_TARGET_BLEND_DESC rt{};
        rt.BlendEnable = TRUE;
        rt.SrcBlend = D3D11_BLEND_SRC_ALPHA;
        rt.DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
        rt.BlendOp = D3D11_BLEND_OP_ADD;
        rt.SrcBlendAlpha = D3D11_BLEND_ONE;
        rt.DestBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA;
        rt.BlendOpAlpha = D3D11_BLEND_OP_ADD;
        rt.RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

        bd.RenderTarget[0] = rt;

        hr = device->CreateBlendState(&bd, m_blend.ReleaseAndGetAddressOf());
        return SUCCEEDED(hr);
    }

    void SpriteRenderer2D::FlushDraw(const Texture2D& tex, int vertexCount)
    {
        // Bind texture SRV (PS t0)
        ID3D11ShaderResourceView* srv = tex.GetSRV();
        m_context->PSSetShaderResources(0, 1, &srv);

        // Issue draw call
        m_context->Draw(vertexCount, 0);
    }

}