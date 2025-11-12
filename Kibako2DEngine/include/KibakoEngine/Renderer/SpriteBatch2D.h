// Kibako2DEngine/include/KibakoEngine/Renderer/SpriteBatch2D.h
#pragma once

#include <cstdint>
#include <vector>

#include <DirectXMath.h>
#include <d3d11.h>
#include <wrl/client.h>

#include "KibakoEngine/Renderer/SpriteTypes.h"
#include "KibakoEngine/Renderer/Texture2D.h"

namespace KibakoEngine {

    using Microsoft::WRL::ComPtr;

    // Collects many sprites and draws them together.
    class SpriteBatch2D {
    public:
        bool Init(ID3D11Device* device, ID3D11DeviceContext* context);
        void Shutdown();

        void Begin(const DirectX::XMFLOAT4X4& viewProj);
        void End();

        void SetMonochrome(float amount) { m_monochrome = amount; }
        void SetPointSampling(bool enable) { m_pointSampling = enable; }
        void SetPixelSnap(bool enable) { m_pixelSnap = enable; }

        void Push(const Texture2D& tex,
                  const RectF& dst,
                  const RectF& src,
                  const Color4& color,
                  float rotation = 0.0f,
                  int layer = 0);

    private:
        struct Vertex {
            DirectX::XMFLOAT3 pos;
            DirectX::XMFLOAT2 uv;
            DirectX::XMFLOAT4 color;
        };

        struct CB_VS_Transform {
            DirectX::XMFLOAT4X4 ViewProj;
        };

        struct CB_PS_Params {
            float Monochrome;
            DirectX::XMFLOAT3 _pad;
        };

        struct DrawCmd {
            const Texture2D* tex = nullptr;
            ID3D11ShaderResourceView* srv = nullptr;
            RectF dst{};
            RectF src{};
            Color4 color{ 1, 1, 1, 1 };
            float rotation = 0.0f;
            int layer = 0;
        };

        bool CreateShaders(ID3D11Device* device);
        bool CreateStates(ID3D11Device* device);
        bool EnsureVB(size_t spriteCapacity);
        bool EnsureIB(size_t spriteCapacity);
        void UpdateVSConstants();
        void UpdatePSConstants();
        void BuildVertsForRange(const DrawCmd* cmds, size_t count, std::vector<Vertex>& outVerts);

    private:
        ID3D11Device* m_device = nullptr;
        ID3D11DeviceContext* m_context = nullptr;

        ComPtr<ID3D11VertexShader> m_vs;
        ComPtr<ID3D11PixelShader> m_ps;
        ComPtr<ID3D11InputLayout> m_inputLayout;

        ComPtr<ID3D11Buffer> m_vb;
        ComPtr<ID3D11Buffer> m_ib;
        size_t m_vbSpriteCap = 0;
        size_t m_ibSpriteCap = 0;

        ComPtr<ID3D11Buffer> m_cbVS;
        ComPtr<ID3D11Buffer> m_cbPS;

        ComPtr<ID3D11SamplerState> m_sampPoint;
        ComPtr<ID3D11SamplerState> m_sampLinear;
        ComPtr<ID3D11BlendState> m_blendAlpha;
        ComPtr<ID3D11DepthStencilState> m_dssOff;
        ComPtr<ID3D11RasterizerState> m_rsCullNone;

        std::vector<DrawCmd> m_queue;
        std::vector<Vertex> m_vertexScratch;
        std::vector<uint32_t> m_indexScratch;
        DirectX::XMFLOAT4X4 m_viewProj{};
        float m_monochrome = 0.0f;
        bool m_pointSampling = true;
        bool m_pixelSnap = true;
        bool m_isDrawing = false;
    };

} // namespace KibakoEngine
