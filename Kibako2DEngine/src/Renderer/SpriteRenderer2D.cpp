// Kibako2DEngine/src/Renderer/SpriteRenderer2D.cpp
#define WIN32_LEAN_AND_MEAN
#include "KibakoEngine/Renderer/SpriteRenderer2D.h"

#include <d3dcompiler.h>
#include <iostream>
#include <cstring>

#pragma comment(lib, "d3dcompiler.lib")

using namespace DirectX;

namespace KibakoEngine {

    // =====================================================
    // INIT & SHUTDOWN
    // =====================================================
    bool SpriteRenderer2D::Init(ID3D11Device* device, ID3D11DeviceContext* context)
    {
        m_device = device;
        m_context = context;

        if (!m_device || !m_context)
        {
            std::cerr << "SpriteRenderer2D::Init received null device/context\n";
            return false;
        }

        if (!CreateShaders(device)) return false;
        if (!CreateBuffers(device)) return false;

        // Create a sampler for texture sampling (required by the pixel shader)
        D3D11_SAMPLER_DESC sd{};
        sd.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;      // change to POINT for sharp pixel-art
        sd.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;           // CLAMP avoids edge bleeding
        sd.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
        sd.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
        sd.MaxLOD = D3D11_FLOAT32_MAX;

        HRESULT hr = m_device->CreateSamplerState(&sd, m_sampler.ReleaseAndGetAddressOf());
        if (FAILED(hr))
        {
            std::cerr << "CreateSamplerState failed (hr=0x" << std::hex << hr << ")\n";
            return false;
        }

        return true;
    }

    void SpriteRenderer2D::Shutdown()
    {
        m_vb.Reset();
        m_cbVS.Reset();
        m_cbPS.Reset();
        m_sampler.Reset();
        m_inputLayout.Reset();
        m_vs.Reset();
        m_ps.Reset();

        m_device = nullptr;
        m_context = nullptr;
        m_isDrawing = false;
    }

    // =====================================================
    // FRAME CONTROL
    // =====================================================
    void SpriteRenderer2D::Begin(const XMFLOAT4X4& viewProj)
    {
        if (m_isDrawing || !m_context) return;
        m_isDrawing = true;
        m_viewProj = viewProj;

        // VS constant buffer (camera)
        D3D11_MAPPED_SUBRESOURCE map{};
        if (SUCCEEDED(m_context->Map(m_cbVS.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &map)))
        {
            auto* cb = reinterpret_cast<CB_VS_Transform*>(map.pData);
            cb->ViewProj = viewProj;
            m_context->Unmap(m_cbVS.Get(), 0);
        }

        ID3D11Buffer* cbVS = m_cbVS.Get();
        m_context->VSSetConstantBuffers(0, 1, &cbVS);
    }

    void SpriteRenderer2D::End()
    {
        if (!m_isDrawing) return;
        m_isDrawing = false;
    }

    // =====================================================
    // DRAW SPRITE
    // =====================================================
    void SpriteRenderer2D::DrawSprite(const Texture2D& tex,
        const RectF& dst,
        const RectF& src,
        const Color4& color,
        float rotation)
    {
        if (!m_isDrawing || !m_context || !tex.GetSRV()) return;

        // PS constants (monochrome toggle)
        D3D11_MAPPED_SUBRESOURCE map{};
        if (SUCCEEDED(m_context->Map(m_cbPS.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &map)))
        {
            auto* cb = reinterpret_cast<CB_PS_Params*>(map.pData);
            cb->Monochrome = m_monochrome;
            m_context->Unmap(m_cbPS.Get(), 0);
        }
        ID3D11Buffer* cbPS = m_cbPS.Get();
        m_context->PSSetConstantBuffers(0, 1, &cbPS);

        // Compute quad corners (rotation around center)
        XMFLOAT2 center{ dst.x + dst.w * 0.5f, dst.y + dst.h * 0.5f };
        XMFLOAT2 corners[4] = {
            { dst.x,         dst.y },
            { dst.x + dst.w, dst.y },
            { dst.x + dst.w, dst.y + dst.h },
            { dst.x,         dst.y + dst.h }
        };

        if (rotation != 0.0f)
        {
            const float c = std::cos(rotation);
            const float s = std::sin(rotation);
            for (auto& v : corners)
            {
                const float dx = v.x - center.x;
                const float dy = v.y - center.y;
                v.x = center.x + dx * c - dy * s;
                v.y = center.y + dx * s + dy * c;
            }
        }

        // UVs from src (0..1)
        const float u1 = src.x;
        const float v1 = src.y;
        const float u2 = src.x + src.w;
        const float v2 = src.y + src.h;

        // Two triangles (clockwise)
        Vertex verts[6];
        verts[0] = { { corners[0].x, corners[0].y, 0.0f }, { u1, v1 }, { color.r, color.g, color.b, color.a } };
        verts[1] = { { corners[1].x, corners[1].y, 0.0f }, { u2, v1 }, { color.r, color.g, color.b, color.a } };
        verts[2] = { { corners[2].x, corners[2].y, 0.0f }, { u2, v2 }, { color.r, color.g, color.b, color.a } };
        verts[3] = { { corners[0].x, corners[0].y, 0.0f }, { u1, v1 }, { color.r, color.g, color.b, color.a } };
        verts[4] = { { corners[2].x, corners[2].y, 0.0f }, { u2, v2 }, { color.r, color.g, color.b, color.a } };
        verts[5] = { { corners[3].x, corners[3].y, 0.0f }, { u1, v2 }, { color.r, color.g, color.b, color.a } };

        m_context->UpdateSubresource(m_vb.Get(), 0, nullptr, verts, 0, 0);

        // Bind pipeline pieces
        UINT stride = sizeof(Vertex);
        UINT offset = 0;
        ID3D11Buffer* vb = m_vb.Get();
        m_context->IASetVertexBuffers(0, 1, &vb, &stride, &offset);
        m_context->IASetInputLayout(m_inputLayout.Get());
        m_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        m_context->VSSetShader(m_vs.Get(), nullptr, 0);
        m_context->PSSetShader(m_ps.Get(), nullptr, 0);

        FlushDraw(tex, 6);
    }

    // =====================================================
    // INTERNAL CREATION HELPERS
    // =====================================================
    bool SpriteRenderer2D::CreateShaders(ID3D11Device* device)
    {
        // Minimal unlit textured pipeline with optional monochrome tint
        const char* vsCode = R"(
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

        const char* psCode = R"(
Texture2D tex0 : register(t0);
SamplerState samp0 : register(s0);
cbuffer CB_PS_Params : register(b0)
{
    float Monochrome;
    float3 pad;
};
float4 mainPS(float4 pos : SV_Position, float2 uv : TEXCOORD, float4 col : COLOR) : SV_Target
{
    float4 texColor = tex0.Sample(samp0, uv);
    float luminance = dot(texColor.rgb, float3(0.299, 0.587, 0.114));
    float3 rgb = lerp(texColor.rgb, float3(luminance, luminance, luminance), Monochrome);
    return float4(rgb * col.rgb, texColor.a * col.a);
})";

        ComPtr<ID3DBlob> vsBlob, psBlob, err;

        HRESULT hr = D3DCompile(vsCode, std::strlen(vsCode), nullptr, nullptr, nullptr,
            "mainVS", "vs_5_0", 0, 0, vsBlob.GetAddressOf(), err.GetAddressOf());
        if (FAILED(hr))
        {
            if (err) std::cerr << "VS compile error: " << (const char*)err->GetBufferPointer() << "\n";
            return false;
        }

        err.Reset();
        hr = D3DCompile(psCode, std::strlen(psCode), nullptr, nullptr, nullptr,
            "mainPS", "ps_5_0", 0, 0, psBlob.GetAddressOf(), err.GetAddressOf());
        if (FAILED(hr))
        {
            if (err) std::cerr << "PS compile error: " << (const char*)err->GetBufferPointer() << "\n";
            return false;
        }

        hr = device->CreateVertexShader(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), nullptr, m_vs.ReleaseAndGetAddressOf());
        if (FAILED(hr)) { std::cerr << "CreateVertexShader failed (hr=0x" << std::hex << hr << ")\n"; return false; }

        hr = device->CreatePixelShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(), nullptr, m_ps.ReleaseAndGetAddressOf());
        if (FAILED(hr)) { std::cerr << "CreatePixelShader failed (hr=0x" << std::hex << hr << ")\n"; return false; }

        D3D11_INPUT_ELEMENT_DESC layout[] = {
            { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,     0,  0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
            { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,        0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
            { "COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT,  0, 20, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        };

        hr = device->CreateInputLayout(layout, _countof(layout),
            vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(),
            m_inputLayout.ReleaseAndGetAddressOf());
        if (FAILED(hr)) { std::cerr << "CreateInputLayout failed (hr=0x" << std::hex << hr << ")\n"; return false; }

        return true;
    }

    bool SpriteRenderer2D::CreateBuffers(ID3D11Device* device)
    {
        // Dynamic quad buffer (we rewrite per sprite)
        D3D11_BUFFER_DESC vbDesc{};
        vbDesc.ByteWidth = sizeof(Vertex) * 6;
        vbDesc.Usage = D3D11_USAGE_DEFAULT;
        vbDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

        HRESULT hr = device->CreateBuffer(&vbDesc, nullptr, m_vb.ReleaseAndGetAddressOf());
        if (FAILED(hr)) { std::cerr << "CreateBuffer(VB) failed (hr=0x" << std::hex << hr << ")\n"; return false; }

        // VS constant buffer (camera)
        D3D11_BUFFER_DESC cbDesc{};
        cbDesc.ByteWidth = sizeof(CB_VS_Transform);
        cbDesc.Usage = D3D11_USAGE_DYNAMIC;
        cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
        cbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

        hr = device->CreateBuffer(&cbDesc, nullptr, m_cbVS.ReleaseAndGetAddressOf());
        if (FAILED(hr)) { std::cerr << "CreateBuffer(CB_VS) failed (hr=0x" << std::hex << hr << ")\n"; return false; }

        // PS constant buffer (params)
        cbDesc.ByteWidth = sizeof(CB_PS_Params);
        hr = device->CreateBuffer(&cbDesc, nullptr, m_cbPS.ReleaseAndGetAddressOf());
        if (FAILED(hr)) { std::cerr << "CreateBuffer(CB_PS) failed (hr=0x" << std::hex << hr << ")\n"; return false; }

        return true;
    }

    void SpriteRenderer2D::FlushDraw(const Texture2D& tex, int vertexCount)
    {
        if (!m_context || !tex.GetSRV()) return;

        ID3D11ShaderResourceView* srv = tex.GetSRV();
        m_context->PSSetShaderResources(0, 1, &srv);

        ID3D11SamplerState* smp = m_sampler.Get();
        m_context->PSSetSamplers(0, 1, &smp);

        m_context->Draw(vertexCount, 0);
    }

}