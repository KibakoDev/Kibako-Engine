// Kibako2DEngine/src/Renderer/SpriteBatch2D.cpp
#define WIN32_LEAN_AND_MEAN
#include "KibakoEngine/Renderer/SpriteBatch2D.h"

#include <algorithm>
#include <cmath>
#include <cstring>
#include <d3dcompiler.h>
#include <iostream>

#include "KibakoEngine/Core/Debug.h"
#include "KibakoEngine/Core/Log.h"

#pragma comment(lib, "d3dcompiler.lib")

using namespace DirectX;

namespace KibakoEngine {

    bool SpriteBatch2D::Init(ID3D11Device* device, ID3D11DeviceContext* context)
    {
        KBK_ASSERT(device != nullptr, "SpriteBatch2D::Init needs device");
        KBK_ASSERT(context != nullptr, "SpriteBatch2D::Init needs context");

        m_device = device;
        m_context = context;

        if (!CreateShaders(device)) return false;
        if (!CreateStates(device))  return false;

        if (!EnsureVB(256))  return false;
        if (!EnsureIB(256))  return false;

        KbkLog("Batch2D", "Init OK (pointSampling=%d, pixelSnap=%d)", (int)m_pointSampling, (int)m_pixelSnap);
        return true;
    }

    void SpriteBatch2D::Shutdown()
    {
        KbkLog("Batch2D", "Shutdown");
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
        m_vbSpriteCap = 0;
        m_ibSpriteCap = 0;
        m_vertexScratch.clear();
        m_indexScratch.clear();
    }

    void SpriteBatch2D::Begin(const XMFLOAT4X4& viewProj)
    {
        KBK_ASSERT(!m_isDrawing, "Begin called twice without End");
        m_isDrawing = true;
        m_viewProj = viewProj;
        m_queue.clear();
        if (m_vbSpriteCap > 0)
            m_queue.reserve(m_vbSpriteCap); // keep one allocation for the frame's sprites
    }

    void SpriteBatch2D::End()
    {
        KBK_ASSERT(m_isDrawing, "End called without Begin");
        m_isDrawing = false;
        if (m_queue.empty()) return;

        m_queue.erase(std::remove_if(m_queue.begin(), m_queue.end(), [](const DrawCmd& cmd) {
            return cmd.tex == nullptr || cmd.srv == nullptr;
        }), m_queue.end());
        if (m_queue.empty()) return;

        const size_t spriteCount = m_queue.size();
        if (spriteCount > 1) {
            // Keep lower layers first and still group sprites by texture within each layer.
            std::stable_sort(m_queue.begin(), m_queue.end(), [](const DrawCmd& a, const DrawCmd& b) {
                if (a.layer != b.layer)
                    return a.layer < b.layer;
                return a.srv < b.srv;
            });
        }

        if (!EnsureVB(spriteCount)) return;
        if (!EnsureIB(spriteCount)) return;

        BuildVertsForRange(m_queue.data(), spriteCount, m_vertexScratch);

        D3D11_MAPPED_SUBRESOURCE map{};
        HRESULT hr = m_context->Map(m_vb.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &map);
        KBK_HR(hr);
        if (FAILED(hr)) return;
        std::memcpy(map.pData, m_vertexScratch.data(), m_vertexScratch.size() * sizeof(Vertex));
        m_context->Unmap(m_vb.Get(), 0);

        UpdateVSConstants();
        UpdatePSConstants();

        ID3D11Buffer* cbVS = m_cbVS.Get();
        ID3D11Buffer* cbPS = m_cbPS.Get();
        m_context->VSSetConstantBuffers(0, 1, &cbVS);
        m_context->PSSetConstantBuffers(0, 1, &cbPS);

        m_context->IASetInputLayout(m_inputLayout.Get());
        m_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

        float blendFactor[4] = { 0,0,0,0 };
        m_context->OMSetBlendState(m_blendAlpha.Get(), blendFactor, 0xFFFFFFFF);
        m_context->OMSetDepthStencilState(m_dssOff.Get(), 0);
        m_context->RSSetState(m_rsCullNone.Get());

        ID3D11SamplerState* samp = m_pointSampling ? m_sampPoint.Get() : m_sampLinear.Get();
        m_context->PSSetSamplers(0, 1, &samp);

        m_context->VSSetShader(m_vs.Get(), nullptr, 0);
        m_context->PSSetShader(m_ps.Get(), nullptr, 0);

        UINT stride = sizeof(Vertex), offset = 0;
        ID3D11Buffer* vb = m_vb.Get();
        m_context->IASetVertexBuffers(0, 1, &vb, &stride, &offset);
        ID3D11Buffer* ib = m_ib.Get();
        m_context->IASetIndexBuffer(ib, DXGI_FORMAT_R32_UINT, 0);

        size_t totalSprites = 0;
        size_t batchCount = 0;

        size_t start = 0;
        while (start < spriteCount) {
            ID3D11ShaderResourceView* srv = m_queue[start].srv;
            if (!srv) {
                ++start;
                continue;
            }

            const int layer = m_queue[start].layer;
            size_t end = start + 1;
            while (end < spriteCount) {
                // stop the bucket when layer or texture changes so we keep the right draw order
                if (m_queue[end].layer != layer || m_queue[end].srv != srv)
                    break;
                ++end;
            }

            const size_t bucketCount = end - start;
            m_context->PSSetShaderResources(0, 1, &srv);

            const UINT indexCount = static_cast<UINT>(bucketCount * 6);
            const UINT indexOffset = static_cast<UINT>(start * 6);
            m_context->DrawIndexed(indexCount, indexOffset, 0);
            totalSprites += bucketCount;
            ++batchCount;

            ID3D11ShaderResourceView* nullSRV[1] = { nullptr };
            m_context->PSSetShaderResources(0, 1, nullSRV);

            start = end;
        }

        KbkLog("Batch2D", "Flushed %zu sprites in %zu batches", totalSprites, batchCount);
    }

    void SpriteBatch2D::Push(const Texture2D& tex,
        const RectF& dst,
        const RectF& src,
        const Color4& color,
        float rotation,
        int   layer)
    {
        if (!m_isDrawing) return;
        ID3D11ShaderResourceView* srv = tex.GetSRV();
        if (!srv)  return;

        DrawCmd cmd{};
        cmd.tex = &tex;
        cmd.srv = srv;
        cmd.dst = dst;
        cmd.src = src;
        cmd.color = color;
        cmd.rotation = rotation;
        cmd.layer = layer;
        m_queue.push_back(cmd);
    }

    bool SpriteBatch2D::CreateShaders(ID3D11Device* device)
    {
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

        Microsoft::WRL::ComPtr<ID3DBlob> vsBlob, psBlob, err;

        HRESULT hr = D3DCompile(vsCode, strlen(vsCode), nullptr, nullptr, nullptr,
            "mainVS", "vs_5_0", 0, 0, vsBlob.GetAddressOf(), err.GetAddressOf());
        KBK_HR(hr);
        if (FAILED(hr)) {
            if (err) std::cerr << "VS compile error: " << (const char*)err->GetBufferPointer() << "\n";
            return false;
        }
        err.Reset();

        hr = D3DCompile(psCode, strlen(psCode), nullptr, nullptr, nullptr,
            "mainPS", "ps_5_0", 0, 0, psBlob.GetAddressOf(), err.GetAddressOf());
        KBK_HR(hr);
        if (FAILED(hr)) {
            if (err) std::cerr << "PS compile error: " << (const char*)err->GetBufferPointer() << "\n";
            return false;
        }

        hr = device->CreateVertexShader(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(),
            nullptr, m_vs.ReleaseAndGetAddressOf());
        KBK_HR(hr); if (FAILED(hr)) return false;

        hr = device->CreatePixelShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(),
            nullptr, m_ps.ReleaseAndGetAddressOf());
        KBK_HR(hr); if (FAILED(hr)) return false;

        D3D11_INPUT_ELEMENT_DESC layout[] = {
            { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,     0,  0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
            { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,        0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
            { "COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT,  0, 20, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        };
        hr = device->CreateInputLayout(layout, _countof(layout),
            vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(),
            m_inputLayout.ReleaseAndGetAddressOf());
        KBK_HR(hr); if (FAILED(hr)) return false;

        D3D11_BUFFER_DESC cbd{};
        cbd.Usage = D3D11_USAGE_DYNAMIC;
        cbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
        cbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

        cbd.ByteWidth = sizeof(CB_VS_Transform);
        hr = device->CreateBuffer(&cbd, nullptr, m_cbVS.ReleaseAndGetAddressOf());
        KBK_HR(hr); if (FAILED(hr)) return false;

        cbd.ByteWidth = sizeof(CB_PS_Params);
        hr = device->CreateBuffer(&cbd, nullptr, m_cbPS.ReleaseAndGetAddressOf());
        KBK_HR(hr); if (FAILED(hr)) return false;

        return true;
    }

    bool SpriteBatch2D::CreateStates(ID3D11Device* device)
    {
        D3D11_SAMPLER_DESC sd{};
        sd.AddressU = sd.AddressV = sd.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
        sd.MaxAnisotropy = 1;
        sd.MinLOD = 0; sd.MaxLOD = D3D11_FLOAT32_MAX;

        sd.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
        HRESULT hr = device->CreateSamplerState(&sd, m_sampPoint.ReleaseAndGetAddressOf());
        KBK_HR(hr); if (FAILED(hr)) return false;

        sd.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
        hr = device->CreateSamplerState(&sd, m_sampLinear.ReleaseAndGetAddressOf());
        KBK_HR(hr); if (FAILED(hr)) return false;

        D3D11_BLEND_DESC bd{};
        bd.RenderTarget[0].BlendEnable = TRUE;
        bd.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
        bd.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
        bd.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
        bd.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
        bd.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA;
        bd.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
        bd.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
        hr = device->CreateBlendState(&bd, m_blendAlpha.ReleaseAndGetAddressOf());
        KBK_HR(hr); if (FAILED(hr)) return false;

        D3D11_DEPTH_STENCIL_DESC dsd{};
        dsd.DepthEnable = FALSE;
        dsd.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
        dsd.DepthFunc = D3D11_COMPARISON_ALWAYS;
        hr = device->CreateDepthStencilState(&dsd, m_dssOff.ReleaseAndGetAddressOf());
        KBK_HR(hr); if (FAILED(hr)) return false;

        D3D11_RASTERIZER_DESC rd{};
        rd.FillMode = D3D11_FILL_SOLID;
        rd.CullMode = D3D11_CULL_NONE;
        rd.DepthClipEnable = TRUE;
        hr = device->CreateRasterizerState(&rd, m_rsCullNone.ReleaseAndGetAddressOf());
        KBK_HR(hr); if (FAILED(hr)) return false;

        return true;
    }

    bool SpriteBatch2D::EnsureVB(size_t spriteCapacity)
    {
        if (spriteCapacity <= m_vbSpriteCap) return true;
        size_t newCap = m_vbSpriteCap ? m_vbSpriteCap : 256;
        while (newCap < spriteCapacity) newCap *= 2;
        m_vbSpriteCap = newCap;

        D3D11_BUFFER_DESC bd{};
        bd.Usage = D3D11_USAGE_DYNAMIC;
        bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
        bd.ByteWidth = static_cast<UINT>(m_vbSpriteCap * 4 * sizeof(Vertex)); // 4 verts / sprite

        Microsoft::WRL::ComPtr<ID3D11Buffer> newVB;
        HRESULT hr = m_device->CreateBuffer(&bd, nullptr, newVB.ReleaseAndGetAddressOf());
        KBK_HR(hr); if (FAILED(hr)) return false;

        m_vb = newVB;
        m_vertexScratch.clear();
        m_vertexScratch.reserve(m_vbSpriteCap * 4);
        return true;
    }

    bool SpriteBatch2D::EnsureIB(size_t spriteCapacity)
    {
        if (m_ib && spriteCapacity <= m_ibSpriteCap) return true;

        const size_t oldCap = m_ibSpriteCap;
        size_t newCap = oldCap ? oldCap : 256;
        while (newCap < spriteCapacity) newCap *= 2;

        if (m_ib && newCap == oldCap) return true;

        m_ibSpriteCap = newCap;
        const size_t requiredIndexCount = m_ibSpriteCap * 6;
        if (m_indexScratch.size() < requiredIndexCount)
            m_indexScratch.resize(requiredIndexCount);

        const size_t startFill = oldCap;
        for (size_t i = startFill; i < m_ibSpriteCap; ++i) {
            const uint32_t base = static_cast<uint32_t>(i * 4);
            uint32_t* idx = m_indexScratch.data() + i * 6;
            idx[0] = base + 0; idx[1] = base + 1; idx[2] = base + 2;
            idx[3] = base + 0; idx[4] = base + 2; idx[5] = base + 3;
        }

        D3D11_BUFFER_DESC bd{};
        bd.Usage = D3D11_USAGE_IMMUTABLE;
        bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
        bd.ByteWidth = static_cast<UINT>(requiredIndexCount * sizeof(uint32_t));

        D3D11_SUBRESOURCE_DATA srd{};
        srd.pSysMem = m_indexScratch.data();

        Microsoft::WRL::ComPtr<ID3D11Buffer> newIB;
        HRESULT hr = m_device->CreateBuffer(&bd, &srd, newIB.ReleaseAndGetAddressOf());
        KBK_HR(hr); if (FAILED(hr)) return false;

        m_ib = newIB;
        return true;
    }

    void SpriteBatch2D::UpdateVSConstants()
    {
        D3D11_MAPPED_SUBRESOURCE map{};
        HRESULT hr = m_context->Map(m_cbVS.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &map);
        KBK_HR(hr);
        if (FAILED(hr)) return;

        std::memcpy(map.pData, &m_viewProj, sizeof(XMFLOAT4X4));
        m_context->Unmap(m_cbVS.Get(), 0);
    }

    void SpriteBatch2D::UpdatePSConstants()
    {
        D3D11_MAPPED_SUBRESOURCE map{};
        HRESULT hr = m_context->Map(m_cbPS.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &map);
        KBK_HR(hr);
        if (FAILED(hr)) return;

        auto* cb = reinterpret_cast<CB_PS_Params*>(map.pData);
        cb->Monochrome = m_monochrome;
        m_context->Unmap(m_cbPS.Get(), 0);
    }

    void SpriteBatch2D::BuildVertsForRange(const DrawCmd* cmds, size_t count,
        std::vector<Vertex>& outVerts)
    {
        const size_t needed = count * 4;
        if (outVerts.capacity() < needed)
            outVerts.reserve(needed);
        outVerts.resize(needed);
        size_t vi = 0;

        for (size_t ci = 0; ci < count; ++ci) {
            const DrawCmd& cmd = cmds[ci];
            const RectF& d = cmd.dst;
            const RectF& s = cmd.src;
            const Color4& c = cmd.color;

            XMFLOAT2 p[4] = {
                { d.x,         d.y          },
                { d.x + d.w,   d.y          },
                { d.x + d.w,   d.y + d.h    },
                { d.x,         d.y + d.h    }
            };

            if (cmd.rotation != 0.0f) {
                const float cx = d.x + d.w * 0.5f;
                const float cy = d.y + d.h * 0.5f;
                const float cs = std::cos(cmd.rotation);
                const float sn = std::sin(cmd.rotation);
                for (int i = 0; i < 4; ++i) {
                    const float dx = p[i].x - cx;
                    const float dy = p[i].y - cy;
                    const float rx = dx * cs - dy * sn;
                    const float ry = dx * sn + dy * cs;
                    p[i].x = cx + rx;
                    p[i].y = cy + ry;
                }
            }

            if (m_pixelSnap && cmd.rotation == 0.0f) {
                for (int i = 0; i < 4; ++i) {
                    p[i].x = std::roundf(p[i].x);
                    p[i].y = std::roundf(p[i].y);
                }
            }

            const float u1 = s.x, v1 = s.y, u2 = s.x + s.w, v2 = s.y + s.h;

            outVerts[vi + 0] = Vertex{ { p[0].x, p[0].y, 0.0f }, { u1, v1 }, { c.r, c.g, c.b, c.a } };
            outVerts[vi + 1] = Vertex{ { p[1].x, p[1].y, 0.0f }, { u2, v1 }, { c.r, c.g, c.b, c.a } };
            outVerts[vi + 2] = Vertex{ { p[2].x, p[2].y, 0.0f }, { u2, v2 }, { c.r, c.g, c.b, c.a } };
            outVerts[vi + 3] = Vertex{ { p[3].x, p[3].y, 0.0f }, { u1, v2 }, { c.r, c.g, c.b, c.a } };

            vi += 4;
        }
    }

}
