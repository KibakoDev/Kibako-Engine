// Kibako2DEngine/src/Renderer/SpriteBatch2D.cpp
#define WIN32_LEAN_AND_MEAN
#include "KibakoEngine/Renderer/SpriteBatch2D.h"
#include <d3dcompiler.h>
#include <cmath>
#include <cstring>
#include <vector>
#include <unordered_map>
#include <iostream>

#pragma comment(lib, "d3dcompiler.lib")

using namespace DirectX;

namespace KibakoEngine {

    // ==============================
    // Init / Shutdown
    // ==============================
    bool SpriteBatch2D::Init(ID3D11Device* device, ID3D11DeviceContext* context)
    {
        m_device = device;
        m_context = context;
        if (!CreateShaders(device)) return false;
        if (!CreateStates(device))  return false;

        // Small initial buffers, will grow as needed
        if (!EnsureVB(256))  return false;
        if (!EnsureIB(256))  return false;

        return true;
    }

    void SpriteBatch2D::Shutdown()
    {
        m_ib.Reset();
        m_vb.Reset();
        m_cbVS.Reset();
        m_cbPS.Reset();
        m_inputLayout.Reset();
        m_vs.Reset();
        m_ps.Reset();
        m_sampPoint.Reset();
        m_sampLinear.Reset();
        m_blendAlpha.Reset();
        m_dssOff.Reset();
        m_rsCullNone.Reset();
        m_device = nullptr;
        m_context = nullptr;
        m_queue.clear();
        m_vbSpriteCap = m_ibSpriteCap = 0;
    }

    // ==============================
    // Frame control
    // ==============================
    void SpriteBatch2D::Begin(const XMFLOAT4X4& viewProj)
    {
        if (m_isDrawing) return;
        m_isDrawing = true;
        m_viewProj = viewProj;
        m_queue.clear();
    }

    void SpriteBatch2D::End()
    {
        if (!m_isDrawing) return;
        m_isDrawing = false;
        if (m_queue.empty()) return;

        // Group by texture (SRV pointer)
        std::unordered_map<ID3D11ShaderResourceView*, std::vector<DrawCmd*>> buckets;
        buckets.reserve(m_queue.size());

        for (auto& cmd : m_queue) {
            auto* srv = cmd.tex ? cmd.tex->GetSRV() : nullptr;
            if (!srv) continue;
            buckets[srv].push_back(&cmd);
        }

        // Prepare pipeline fixed state
        UpdateVSConstants();
        UpdatePSConstants();

        // Common pipeline state
        ID3D11Buffer* cbVS = m_cbVS.Get();
        ID3D11Buffer* cbPS = m_cbPS.Get();
        m_context->VSSetConstantBuffers(0, 1, &cbVS);
        m_context->PSSetConstantBuffers(0, 1, &cbPS);

        m_context->IASetInputLayout(m_inputLayout.Get());
        m_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

        // States: blend alpha, depth off, cull none
        float blendFactor[4] = { 0,0,0,0 };
        m_context->OMSetBlendState(m_blendAlpha.Get(), blendFactor, 0xFFFFFFFF);
        m_context->OMSetDepthStencilState(m_dssOff.Get(), 0);
        m_context->RSSetState(m_rsCullNone.Get());

        // Sampler
        ID3D11SamplerState* samp = m_pointSampling ? m_sampPoint.Get() : m_sampLinear.Get();
        m_context->PSSetSamplers(0, 1, &samp);

        // Shaders
        m_context->VSSetShader(m_vs.Get(), nullptr, 0);
        m_context->PSSetShader(m_ps.Get(), nullptr, 0);

        // Build and draw per texture
        std::vector<Vertex> cpuVerts;
        for (auto& kv : buckets) {
            auto* srv = kv.first;
            auto& bucket = kv.second;
            if (bucket.empty()) continue;

            // Ensure buffers capacity
            if (!EnsureVB(bucket.size())) return;
            if (!EnsureIB(bucket.size())) return;

            // Build CPU vertices
            cpuVerts.clear();
            cpuVerts.reserve(bucket.size() * 4);
            BuildVertsForBucket(bucket, cpuVerts);

            // Upload VB
            D3D11_MAPPED_SUBRESOURCE map{};
            if (SUCCEEDED(m_context->Map(m_vb.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &map))) {
                std::memcpy(map.pData, cpuVerts.data(), cpuVerts.size() * sizeof(Vertex));
                m_context->Unmap(m_vb.Get(), 0);
            }

            // Bind buffers
            UINT stride = sizeof(Vertex);
            UINT offset = 0;
            ID3D11Buffer* vb = m_vb.Get();
            m_context->IASetVertexBuffers(0, 1, &vb, &stride, &offset);
            ID3D11Buffer* ib = m_ib.Get();
            m_context->IASetIndexBuffer(ib, DXGI_FORMAT_R32_UINT, 0);

            // Bind texture
            m_context->PSSetShaderResources(0, 1, &srv);

            // Draw indexed (6 indices per sprite)
            UINT indexCount = static_cast<UINT>(bucket.size() * 6);
            m_context->DrawIndexed(indexCount, 0, 0);

            // Unbind SRV (avoid hazard if device debug is on)
            ID3D11ShaderResourceView* nullSRV[1] = { nullptr };
            m_context->PSSetShaderResources(0, 1, nullSRV);
        }
    }

    // ==============================
    // Queue a sprite
    // ==============================
    void SpriteBatch2D::Push(const Texture2D& tex,
        const RectF& dst,
        const RectF& src,
        const Color4& color,
        float rotation)
    {
        if (!m_isDrawing) return;
        if (!tex.GetSRV()) return;
        DrawCmd cmd{};
        cmd.tex = &tex;
        cmd.dst = dst;
        cmd.src = src;
        cmd.color = color;
        cmd.rotation = rotation;
        m_queue.push_back(cmd);
    }

    // ==============================
    // Internal helpers
    // ==============================
    bool SpriteBatch2D::CreateShaders(ID3D11Device* device)
    {
        // VS: transform quad and pass uv/color
        const char* vsCode = R"(
cbuffer CB_VS_Transform : register(b0) { float4x4 gViewProj; };
struct VSIn  { float3 pos:POSITION; float2 uv:TEXCOORD; float4 col:COLOR; };
struct VSOut { float4 pos:SV_Position; float2 uv:TEXCOORD; float4 col:COLOR; };
VSOut mainVS(VSIn i){
    VSOut o;
    o.pos = mul(float4(i.pos,1.0), gViewProj);
    o.uv  = i.uv;
    o.col = i.col;
    return o;
})";

        // PS: sample texture, optional grayscale
        const char* psCode = R"(
Texture2D tex0 : register(t0);
SamplerState samp0 : register(s0);
cbuffer CB_PS_Params : register(b0) { float Monochrome; float3 _pad; }
float4 mainPS(float4 pos:SV_Position, float2 uv:TEXCOORD, float4 col:COLOR) : SV_Target {
    float4 t = tex0.Sample(samp0, uv);
    float l = dot(t.rgb, float3(0.299, 0.587, 0.114));
    float3 mixrgb = lerp(t.rgb, l.xxx, saturate(Monochrome));
    return float4(mixrgb * col.rgb, t.a * col.a);
})";

        ComPtr<ID3DBlob> vsBlob, psBlob, err;

        HRESULT hr = D3DCompile(vsCode, strlen(vsCode), nullptr, nullptr, nullptr,
            "mainVS", "vs_5_0", 0, 0, vsBlob.GetAddressOf(), err.GetAddressOf());
        if (FAILED(hr)) {
            if (err) std::cerr << "VS compile error: " << (const char*)err->GetBufferPointer() << "\n";
            return false;
        }
        err.Reset();

        hr = D3DCompile(psCode, strlen(psCode), nullptr, nullptr, nullptr,
            "mainPS", "ps_5_0", 0, 0, psBlob.GetAddressOf(), err.GetAddressOf());
        if (FAILED(hr)) {
            if (err) std::cerr << "PS compile error: " << (const char*)err->GetBufferPointer() << "\n";
            return false;
        }

        hr = device->CreateVertexShader(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(),
            nullptr, m_vs.ReleaseAndGetAddressOf());
        if (FAILED(hr)) return false;
        hr = device->CreatePixelShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(),
            nullptr, m_ps.ReleaseAndGetAddressOf());
        if (FAILED(hr)) return false;

        D3D11_INPUT_ELEMENT_DESC layout[] = {
            { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,     0,  0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
            { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,        0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
            { "COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT,  0, 20, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        };
        hr = device->CreateInputLayout(layout, _countof(layout),
            vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(),
            m_inputLayout.ReleaseAndGetAddressOf());
        if (FAILED(hr)) return false;

        // Constant buffers
        D3D11_BUFFER_DESC cbd{};
        cbd.Usage = D3D11_USAGE_DYNAMIC;
        cbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
        cbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

        cbd.ByteWidth = sizeof(CB_VS_Transform);
        if (FAILED(device->CreateBuffer(&cbd, nullptr, m_cbVS.ReleaseAndGetAddressOf()))) return false;

        cbd.ByteWidth = sizeof(CB_PS_Params);
        if (FAILED(device->CreateBuffer(&cbd, nullptr, m_cbPS.ReleaseAndGetAddressOf()))) return false;

        return true;
    }

    bool SpriteBatch2D::CreateStates(ID3D11Device* device)
    {
        // Samplers
        D3D11_SAMPLER_DESC sd{};
        sd.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
        sd.AddressU = sd.AddressV = sd.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
        sd.MaxAnisotropy = 1;
        sd.MinLOD = 0; sd.MaxLOD = D3D11_FLOAT32_MAX;
        if (FAILED(device->CreateSamplerState(&sd, m_sampPoint.ReleaseAndGetAddressOf()))) return false;

        sd.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
        if (FAILED(device->CreateSamplerState(&sd, m_sampLinear.ReleaseAndGetAddressOf()))) return false;

        // Blend alpha
        D3D11_BLEND_DESC bd{};
        bd.RenderTarget[0].BlendEnable = TRUE;
        bd.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
        bd.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
        bd.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
        bd.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
        bd.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA;
        bd.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
        bd.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
        if (FAILED(device->CreateBlendState(&bd, m_blendAlpha.ReleaseAndGetAddressOf()))) return false;

        // Depth off
        D3D11_DEPTH_STENCIL_DESC dsd{};
        dsd.DepthEnable = FALSE;
        dsd.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
        dsd.DepthFunc = D3D11_COMPARISON_ALWAYS;
        if (FAILED(device->CreateDepthStencilState(&dsd, m_dssOff.ReleaseAndGetAddressOf()))) return false;

        // Rasterizer cull none
        D3D11_RASTERIZER_DESC rd{};
        rd.FillMode = D3D11_FILL_SOLID;
        rd.CullMode = D3D11_CULL_NONE;
        rd.DepthClipEnable = TRUE;
        if (FAILED(device->CreateRasterizerState(&rd, m_rsCullNone.ReleaseAndGetAddressOf()))) return false;

        return true;
    }

    bool SpriteBatch2D::EnsureVB(size_t spriteCapacity)
    {
        if (spriteCapacity <= m_vbSpriteCap) return true;
        // grow to next power of two-ish
        size_t newCap = m_vbSpriteCap ? m_vbSpriteCap : 256;
        while (newCap < spriteCapacity) newCap *= 2;
        m_vbSpriteCap = newCap;

        D3D11_BUFFER_DESC bd{};
        bd.Usage = D3D11_USAGE_DYNAMIC;
        bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
        bd.ByteWidth = static_cast<UINT>(m_vbSpriteCap * 4 * sizeof(Vertex)); // 4 verts per sprite

        ComPtr<ID3D11Buffer> newVB;
        if (FAILED(m_device->CreateBuffer(&bd, nullptr, newVB.ReleaseAndGetAddressOf()))) return false;
        m_vb = newVB;
        return true;
    }

    bool SpriteBatch2D::EnsureIB(size_t spriteCapacity)
    {
        if (spriteCapacity <= m_ibSpriteCap) return true;
        size_t newCap = m_ibSpriteCap ? m_ibSpriteCap : 256;
        while (newCap < spriteCapacity) newCap *= 2;
        m_ibSpriteCap = newCap;

        // Fill index pattern: (0,1,2, 0,2,3) per sprite
        std::vector<uint32_t> indices(m_ibSpriteCap * 6);
        for (size_t i = 0; i < m_ibSpriteCap; ++i) {
            uint32_t base = static_cast<uint32_t>(i * 4);
            uint32_t* idx = indices.data() + i * 6;
            idx[0] = base + 0; idx[1] = base + 1; idx[2] = base + 2;
            idx[3] = base + 0; idx[4] = base + 2; idx[5] = base + 3;
        }

        D3D11_BUFFER_DESC bd{};
        bd.Usage = D3D11_USAGE_IMMUTABLE;
        bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
        bd.ByteWidth = static_cast<UINT>(indices.size() * sizeof(uint32_t));

        D3D11_SUBRESOURCE_DATA srd{};
        srd.pSysMem = indices.data();

        ComPtr<ID3D11Buffer> newIB;
        if (FAILED(m_device->CreateBuffer(&bd, &srd, newIB.ReleaseAndGetAddressOf()))) return false;
        m_ib = newIB;
        return true;
    }

    void SpriteBatch2D::UpdateVSConstants()
    {
        D3D11_MAPPED_SUBRESOURCE map{};
        if (SUCCEEDED(m_context->Map(m_cbVS.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &map))) {
            std::memcpy(map.pData, &m_viewProj, sizeof(XMFLOAT4X4));
            m_context->Unmap(m_cbVS.Get(), 0);
        }
    }

    void SpriteBatch2D::UpdatePSConstants()
    {
        D3D11_MAPPED_SUBRESOURCE map{};
        if (SUCCEEDED(m_context->Map(m_cbPS.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &map))) {
            auto* cb = reinterpret_cast<CB_PS_Params*>(map.pData);
            cb->Monochrome = m_monochrome;
            m_context->Unmap(m_cbPS.Get(), 0);
        }
    }

    void SpriteBatch2D::BuildVertsForBucket(const std::vector<DrawCmd*>& bucket,
        std::vector<Vertex>& outVerts)
    {
        outVerts.resize(bucket.size() * 4);
        size_t vi = 0;

        for (auto* cmd : bucket) {
            const RectF& d = cmd->dst;
            const RectF& s = cmd->src;
            const Color4& c = cmd->color;

            // Corners of the quad
            XMFLOAT2 p[4] = {
                { d.x,       d.y       },
                { d.x + d.w, d.y       },
                { d.x + d.w, d.y + d.h },
                { d.x,       d.y + d.h }
            };

            // Optional rotation around center
            if (cmd->rotation != 0.0f) {
                const float cx = d.x + d.w * 0.5f;
                const float cy = d.y + d.h * 0.5f;
                const float cs = std::cos(cmd->rotation);
                const float sn = std::sin(cmd->rotation);
                for (int i = 0; i < 4; ++i) {
                    const float dx = p[i].x - cx;
                    const float dy = p[i].y - cy;
                    const float rx = dx * cs - dy * sn;
                    const float ry = dx * sn + dy * cs;
                    p[i].x = cx + rx;
                    p[i].y = cy + ry;
                }
            }

            // UVs (avoid names v1/v2 that clash with vertex vars)
            const float uA = s.x;
            const float vA = s.y;
            const float uB = s.x + s.w;
            const float vB = s.y + s.h;

            // Build 4 vertices (order 0-1-2-3 matches the IB pattern)
            Vertex vert0{ { p[0].x, p[0].y, 0.0f }, { uA, vA }, { c.r, c.g, c.b, c.a } };
            Vertex vert1{ { p[1].x, p[1].y, 0.0f }, { uB, vA }, { c.r, c.g, c.b, c.a } };
            Vertex vert2{ { p[2].x, p[2].y, 0.0f }, { uB, vB }, { c.r, c.g, c.b, c.a } };
            Vertex vert3{ { p[3].x, p[3].y, 0.0f }, { uA, vB }, { c.r, c.g, c.b, c.a } };

            outVerts[vi + 0] = vert0;
            outVerts[vi + 1] = vert1;
            outVerts[vi + 2] = vert2;
            outVerts[vi + 3] = vert3;

            vi += 4;
        }
    }
}