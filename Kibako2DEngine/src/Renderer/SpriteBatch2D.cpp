// Batched Direct3D 11 sprite renderer
#include "KibakoEngine/Renderer/SpriteBatch2D.h"

#include "KibakoEngine/Core/Debug.h"
#include "KibakoEngine/Core/Log.h"
#include "KibakoEngine/Core/Profiler.h"

#include <d3dcompiler.h>

#include <algorithm>
#include <cmath>
#include <cstring>
#include <vector>

using namespace DirectX;

#pragma comment(lib, "d3dcompiler.lib")

namespace KibakoEngine {

    namespace
    {
        constexpr const char* kLogChannel = "SpriteBatch";
    }

    const Texture2D* SpriteBatch2D::DefaultWhiteTexture() const
    {
        return m_defaultWhite.IsValid() ? &m_defaultWhite : nullptr;
    }

    bool SpriteBatch2D::Init(ID3D11Device* device, ID3D11DeviceContext* context)
    {
        KBK_PROFILE_SCOPE("SpriteBatchInit");

        KBK_ASSERT(device != nullptr, "SpriteBatch2D::Init requires device");
        KBK_ASSERT(context != nullptr, "SpriteBatch2D::Init requires context");
        m_device = device;
        m_context = context;

        if (!CreateShaders(device)) {
            KbkError(kLogChannel, "Failed to create shaders");
            return false;
        }
        if (!CreateStates(device)) {
            KbkError(kLogChannel, "Failed to create states");
            return false;
        }
        if (!EnsureVertexCapacity(256))
            return false;
        if (!EnsureIndexCapacity(256))
            return false;

        if (!m_defaultWhite.CreateSolidColor(device, 255, 255, 255, 255)) {
            KbkWarn(kLogChannel, "Failed to create default white texture for SpriteBatch2D");
        }

        return true;
    }

    void SpriteBatch2D::Shutdown()
    {
        KBK_PROFILE_SCOPE("SpriteBatchShutdown");

        m_indexScratch.clear();
        m_vertexScratch.clear();
        m_commands.clear();

        m_defaultWhite.Reset();

        m_vertexBuffer.Reset();
        m_indexBuffer.Reset();
        m_cbVS.Reset();
        m_vs.Reset();
        m_ps.Reset();
        m_inputLayout.Reset();
        m_samplerPoint.Reset();
        m_blendAlpha.Reset();
        m_depthDisabled.Reset();
        m_rasterCullNone.Reset();

        m_device = nullptr;
        m_context = nullptr;
        m_vertexCapacitySprites = 0;
        m_indexCapacitySprites = 0;
        m_isDrawing = false;
        m_stats = {};
    }

    void SpriteBatch2D::Begin(const XMFLOAT4X4& viewProjT)
    {
        KBK_PROFILE_SCOPE("SpriteBatchBegin");

        m_stats = {};

        KBK_ASSERT(!m_isDrawing, "SpriteBatch2D::Begin without End");
        m_isDrawing = true;
        m_viewProjT = viewProjT;
        m_commands.clear();
    }

    void SpriteBatch2D::End()
    {
        KBK_PROFILE_SCOPE("SpriteBatchEnd");

        KBK_ASSERT(m_isDrawing, "SpriteBatch2D::End without Begin");
        m_isDrawing = false;

        m_commands.erase(
            std::remove_if(
                m_commands.begin(), m_commands.end(),
                [](const DrawCommand& cmd) {
                    return cmd.texture == nullptr || cmd.texture->GetSRV() == nullptr;
                }),
            m_commands.end()
        );
        if (m_commands.empty())
            return;

        std::stable_sort(
            m_commands.begin(), m_commands.end(),
            [](const DrawCommand& a, const DrawCommand& b) {
                if (a.layer != b.layer)
                    return a.layer < b.layer;
                const auto srvA = a.texture->GetSRV();
                const auto srvB = b.texture->GetSRV();
                return srvA < srvB;
            });

        const size_t spriteCount = m_commands.size();
        if (!EnsureVertexCapacity(spriteCount) || !EnsureIndexCapacity(spriteCount))
            return;

        UpdateVSConstants();

        m_vertexScratch.clear();
        m_vertexScratch.reserve(spriteCount * 4);
        BuildVertices(m_vertexScratch);

        D3D11_MAPPED_SUBRESOURCE mapped{};
        const HRESULT mapResult = m_context->Map(m_vertexBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
        if (FAILED(mapResult)) {
            KbkError(kLogChannel, "Vertex buffer map failed: 0x%08X", static_cast<unsigned>(mapResult));
            return;
        }

        std::memcpy(mapped.pData, m_vertexScratch.data(), m_vertexScratch.size() * sizeof(Vertex));
        m_context->Unmap(m_vertexBuffer.Get(), 0);

        const UINT stride = sizeof(Vertex);
        const UINT offset = 0;
        ID3D11Buffer* vb = m_vertexBuffer.Get();
        ID3D11Buffer* ib = m_indexBuffer.Get();
        m_context->IASetVertexBuffers(0, 1, &vb, &stride, &offset);
        m_context->IASetIndexBuffer(ib, DXGI_FORMAT_R32_UINT, 0);
        m_context->IASetInputLayout(m_inputLayout.Get());
        m_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

        ID3D11Buffer* cbs[] = { m_cbVS.Get() };
        m_context->VSSetConstantBuffers(0, 1, cbs);

        m_context->VSSetShader(m_vs.Get(), nullptr, 0);
        m_context->PSSetShader(m_ps.Get(), nullptr, 0);

        const float blendFactor[4] = { 0.f, 0.f, 0.f, 0.f };
        m_context->OMSetBlendState(m_blendAlpha.Get(), blendFactor, 0xFFFFFFFFu);
        m_context->OMSetDepthStencilState(m_depthDisabled.Get(), 0);
        m_context->RSSetState(m_rasterCullNone.Get());

        ID3D11SamplerState* sampler = m_samplerPoint.Get();
        m_context->PSSetSamplers(0, 1, &sampler);

        size_t start = 0;
        while (start < spriteCount) {
            const DrawCommand& first = m_commands[start];
            ID3D11ShaderResourceView* srv = first.texture->GetSRV();
            size_t end = start + 1;
            while (end < spriteCount) {
                const DrawCommand& next = m_commands[end];
                if (next.layer != first.layer || next.texture->GetSRV() != srv)
                    break;
                ++end;
            }

            m_context->PSSetShaderResources(0, 1, &srv);
            const UINT startIndex = static_cast<UINT>(start * 6);
            const UINT indexCount = static_cast<UINT>((end - start) * 6);
            m_context->DrawIndexed(indexCount, startIndex, 0);

            ID3D11ShaderResourceView* nullSrv = nullptr;
            m_context->PSSetShaderResources(0, 1, &nullSrv);

            m_stats.drawCalls++;

            start = end;
        }
    }

    void SpriteBatch2D::Push(const Texture2D& texture,
        const RectF& dst,
        const RectF& src,
        const Color4& color,
        float rotation,
        int layer)
    {
#if KBK_DEBUG_BUILD
        KBK_ASSERT(m_isDrawing, "SpriteBatch2D::Push called outside Begin/End");
#endif
        if (!m_isDrawing)
            return;

        m_commands.push_back({ &texture, dst, src, color, rotation, layer });
        m_stats.spritesSubmitted++;
    }

    bool SpriteBatch2D::CreateShaders(ID3D11Device* device)
    {
        KBK_PROFILE_SCOPE("CreateBatchShaders");

        static constexpr const char* VS_SOURCE = R"(
cbuffer CB_VS : register(b0)
{
    float4x4 gViewProj;
};

struct VSInput
{
    float3 position : POSITION;
    float2 texcoord : TEXCOORD0;
    float4 color    : COLOR0;
};

struct VSOutput
{
    float4 position : SV_Position;
    float2 texcoord : TEXCOORD0;
    float4 color    : COLOR0;
};

VSOutput main(VSInput input)
{
    VSOutput output;
    output.position = mul(float4(input.position, 1.0f), gViewProj);
    output.texcoord = input.texcoord;
    output.color = input.color;
    return output;
}
)";

        static constexpr const char* PS_SOURCE = R"(
Texture2D gTexture : register(t0);
SamplerState gSampler : register(s0);

float4 main(float4 position : SV_Position, float2 texcoord : TEXCOORD0, float4 color : COLOR0) : SV_Target
{
    float4 texColor = gTexture.Sample(gSampler, texcoord);
    return float4(texColor.rgb * color.rgb, texColor.a * color.a);
}
)";

        Microsoft::WRL::ComPtr<ID3DBlob> vsBlob;
        Microsoft::WRL::ComPtr<ID3DBlob> psBlob;
        Microsoft::WRL::ComPtr<ID3DBlob> errors;

        HRESULT hr = D3DCompile(VS_SOURCE, std::strlen(VS_SOURCE), nullptr, nullptr, nullptr, "main", "vs_5_0", 0, 0,
            vsBlob.GetAddressOf(), errors.GetAddressOf());
        if (FAILED(hr)) {
            if (errors)
                KbkError(kLogChannel, "VS compile error: %s", static_cast<const char*>(errors->GetBufferPointer()));
            return false;
        }
        errors.Reset();

        hr = D3DCompile(PS_SOURCE, std::strlen(PS_SOURCE), nullptr, nullptr, nullptr, "main", "ps_5_0", 0, 0,
            psBlob.GetAddressOf(), errors.GetAddressOf());
        if (FAILED(hr)) {
            if (errors)
                KbkError(kLogChannel, "PS compile error: %s", static_cast<const char*>(errors->GetBufferPointer()));
            return false;
        }

        hr = device->CreateVertexShader(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), nullptr, m_vs.GetAddressOf());
        if (FAILED(hr)) {
            KbkError(kLogChannel, "CreateVertexShader failed: 0x%08X", static_cast<unsigned>(hr));
            return false;
        }
        hr = device->CreatePixelShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(), nullptr, m_ps.GetAddressOf());
        if (FAILED(hr)) {
            KbkError(kLogChannel, "CreatePixelShader failed: 0x%08X", static_cast<unsigned>(hr));
            return false;
        }

        D3D11_INPUT_ELEMENT_DESC layout[] = {
            { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,  D3D11_INPUT_PER_VERTEX_DATA, 0 },
            { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
            { "COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 20, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        };
        hr = device->CreateInputLayout(layout, static_cast<UINT>(std::size(layout)), vsBlob->GetBufferPointer(),
            vsBlob->GetBufferSize(), m_inputLayout.GetAddressOf());
        if (FAILED(hr)) {
            KbkError(kLogChannel, "CreateInputLayout failed: 0x%08X", static_cast<unsigned>(hr));
            return false;
        }

        D3D11_BUFFER_DESC cbd{};
        cbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
        cbd.Usage = D3D11_USAGE_DYNAMIC;
        cbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
        cbd.ByteWidth = sizeof(CBVS);
        hr = device->CreateBuffer(&cbd, nullptr, m_cbVS.GetAddressOf());
        if (FAILED(hr)) {
            KbkError(kLogChannel, "CreateBuffer (CBVS) failed: 0x%08X", static_cast<unsigned>(hr));
            return false;
        }

        return true;
    }

    bool SpriteBatch2D::CreateStates(ID3D11Device* device)
    {
        KBK_PROFILE_SCOPE("CreateBatchStates");

        D3D11_SAMPLER_DESC samp{};
        samp.AddressU = samp.AddressV = samp.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
        samp.MinLOD = 0;
        samp.MaxLOD = D3D11_FLOAT32_MAX;
        samp.MaxAnisotropy = 1;
        samp.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
        HRESULT hr = device->CreateSamplerState(&samp, m_samplerPoint.GetAddressOf());
        if (FAILED(hr)) {
            KbkError(kLogChannel, "CreateSamplerState failed: 0x%08X", static_cast<unsigned>(hr));
            return false;
        }

        D3D11_BLEND_DESC blend{};
        blend.RenderTarget[0].BlendEnable = TRUE;
        blend.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
        blend.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
        blend.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
        blend.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
        blend.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA;
        blend.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
        blend.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
        hr = device->CreateBlendState(&blend, m_blendAlpha.GetAddressOf());
        if (FAILED(hr)) {
            KbkError(kLogChannel, "CreateBlendState failed: 0x%08X", static_cast<unsigned>(hr));
            return false;
        }

        D3D11_DEPTH_STENCIL_DESC depth{};
        depth.DepthEnable = FALSE;
        depth.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
        depth.DepthFunc = D3D11_COMPARISON_ALWAYS;
        hr = device->CreateDepthStencilState(&depth, m_depthDisabled.GetAddressOf());
        if (FAILED(hr)) {
            KbkError(kLogChannel, "CreateDepthStencilState failed: 0x%08X", static_cast<unsigned>(hr));
            return false;
        }

        D3D11_RASTERIZER_DESC rast{};
        rast.FillMode = D3D11_FILL_SOLID;
        rast.CullMode = D3D11_CULL_NONE;
        rast.DepthClipEnable = TRUE;
        hr = device->CreateRasterizerState(&rast, m_rasterCullNone.GetAddressOf());
        if (FAILED(hr)) {
            KbkError(kLogChannel, "CreateRasterizerState failed: 0x%08X", static_cast<unsigned>(hr));
            return false;
        }

        return true;
    }

    bool SpriteBatch2D::EnsureVertexCapacity(size_t spriteCount)
    {
        KBK_PROFILE_SCOPE("EnsureVertexCapacity");

        if (spriteCount <= m_vertexCapacitySprites && m_vertexBuffer)
            return true;

        size_t newCapacity = m_vertexCapacitySprites == 0 ? 256 : m_vertexCapacitySprites;
        while (newCapacity < spriteCount)
            newCapacity *= 2;

        D3D11_BUFFER_DESC desc{};
        desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        desc.Usage = D3D11_USAGE_DYNAMIC;
        desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
        desc.ByteWidth = static_cast<UINT>(newCapacity * 4 * sizeof(Vertex));

        Microsoft::WRL::ComPtr<ID3D11Buffer> buffer;
        const HRESULT hr = m_device->CreateBuffer(&desc, nullptr, buffer.GetAddressOf());
        if (FAILED(hr)) {
            KbkError(kLogChannel, "CreateBuffer (VB) failed: 0x%08X", static_cast<unsigned>(hr));
            return false;
        }

        m_vertexBuffer = buffer;
        m_vertexCapacitySprites = newCapacity;
        return true;
    }

    bool SpriteBatch2D::EnsureIndexCapacity(size_t spriteCount)
    {
        KBK_PROFILE_SCOPE("EnsureIndexCapacity");

        if (spriteCount <= m_indexCapacitySprites && m_indexBuffer)
            return true;

        size_t newCapacity = m_indexCapacitySprites == 0 ? 256 : m_indexCapacitySprites;
        while (newCapacity < spriteCount)
            newCapacity *= 2;

        const size_t indexCount = newCapacity * 6;
        m_indexScratch.resize(indexCount);
        for (size_t sprite = 0; sprite < newCapacity; ++sprite) {
            const std::uint32_t base = static_cast<std::uint32_t>(sprite * 4);
            std::uint32_t* indices = m_indexScratch.data() + sprite * 6;
            indices[0] = base;
            indices[1] = base + 1;
            indices[2] = base + 2;
            indices[3] = base;
            indices[4] = base + 2;
            indices[5] = base + 3;
        }

        D3D11_BUFFER_DESC desc{};
        desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
        desc.Usage = D3D11_USAGE_IMMUTABLE;
        desc.ByteWidth = static_cast<UINT>(indexCount * sizeof(std::uint32_t));

        D3D11_SUBRESOURCE_DATA data{};
        data.pSysMem = m_indexScratch.data();

        Microsoft::WRL::ComPtr<ID3D11Buffer> buffer;
        const HRESULT hr = m_device->CreateBuffer(&desc, &data, buffer.GetAddressOf());
        if (FAILED(hr)) {
            KbkError(kLogChannel, "CreateBuffer (IB) failed: 0x%08X", static_cast<unsigned>(hr));
            return false;
        }

        m_indexBuffer = buffer;
        m_indexCapacitySprites = newCapacity;
        return true;
    }

    void SpriteBatch2D::UpdateVSConstants()
    {
        KBK_PROFILE_SCOPE("UpdateVSConstants");

        D3D11_MAPPED_SUBRESOURCE mapped{};
        const HRESULT hr = m_context->Map(m_cbVS.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
        if (FAILED(hr)) {
            KbkError(kLogChannel, "Constant buffer map failed: 0x%08X", static_cast<unsigned>(hr));
            return;
        }

        auto* cb = static_cast<CBVS*>(mapped.pData);
        cb->viewProjT = m_viewProjT;
        m_context->Unmap(m_cbVS.Get(), 0);
    }

    void SpriteBatch2D::BuildVertices(std::vector<Vertex>& outVertices) const
    {
        KBK_PROFILE_SCOPE("BuildSpriteVertices");

        const size_t spriteCount = m_commands.size();
        outVertices.resize(spriteCount * 4);

        size_t v = 0;
        for (const DrawCommand& cmd : m_commands) {
            const float left = cmd.dst.x;
            const float top = cmd.dst.y;
            const float right = cmd.dst.x + cmd.dst.w;
            const float bottom = cmd.dst.y + cmd.dst.h;

            XMFLOAT2 corners[4] = {
                { left,  top    },
                { right, top    },
                { right, bottom },
                { left,  bottom },
            };

            if (std::fabs(cmd.rotation) > 0.0001f) {
                const float cx = cmd.dst.x + cmd.dst.w * 0.5f;
                const float cy = cmd.dst.y + cmd.dst.h * 0.5f;
                const float cs = std::cos(cmd.rotation);
                const float sn = std::sin(cmd.rotation);
                for (auto& p : corners) {
                    const float dx = p.x - cx;
                    const float dy = p.y - cy;
                    p.x = cx + dx * cs - dy * sn;
                    p.y = cy + dx * sn + dy * cs;
                }
            }

            const float u0 = cmd.src.x;
            const float v0 = cmd.src.y;
            const float u1 = cmd.src.x + cmd.src.w;
            const float v1 = cmd.src.y + cmd.src.h;

            const XMFLOAT4 color = { cmd.color.r, cmd.color.g, cmd.color.b, cmd.color.a };

            outVertices[v + 0] = Vertex{ { corners[0].x, corners[0].y, 0.0f }, { u0, v0 }, color };
            outVertices[v + 1] = Vertex{ { corners[1].x, corners[1].y, 0.0f }, { u1, v0 }, color };
            outVertices[v + 2] = Vertex{ { corners[2].x, corners[2].y, 0.0f }, { u1, v1 }, color };
            outVertices[v + 3] = Vertex{ { corners[3].x, corners[3].y, 0.0f }, { u0, v1 }, color };

            v += 4;
        }
    }

} // namespace KibakoEngine
