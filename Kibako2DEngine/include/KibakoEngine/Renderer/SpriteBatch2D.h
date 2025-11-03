// Kibako2DEngine/include/KibakoEngine/Renderer/SpriteBatch2D.h
#pragma once
#include <d3d11.h>
#include <wrl/client.h>
#include <DirectXMath.h>
#include <vector>
#include <unordered_map>

#include "KibakoEngine/Renderer/Texture2D.h"
#include "KibakoEngine/Renderer/SpriteTypes.h"

namespace KibakoEngine {

    using Microsoft::WRL::ComPtr;
    using namespace DirectX;

    // Batched 2D sprites: Push(...) during a frame, then End() draws by texture buckets
    class SpriteBatch2D {
    public:
        bool Init(ID3D11Device* device, ID3D11DeviceContext* context);
        void Shutdown();

        // Frame
        void Begin(const XMFLOAT4X4& viewProj); // must be called before Push
        void End();                              // builds buffers and issues draws
        bool IsDrawing() const { return m_isDrawing; }

        // Draw request (world pixel units)
        void Push(const Texture2D& tex,
            const RectF& dst,    // x,y,w,h in world pixels
            const RectF& src,    // uv in [0..1]
            const Color4& color,
            float rotation = 0.0f);

        // Global options
        void SetMonochrome(float amount) { m_monochrome = amount; } // 0 = color, 1 = gray
        void SetPointSampling(bool enabled) { m_pointSampling = enabled; }

    private:
        struct Vertex {
            XMFLOAT3 pos;
            XMFLOAT2 uv;
            XMFLOAT4 color;
        };

        struct DrawCmd {
            const Texture2D* tex;
            RectF dst;
            RectF src;
            Color4 color;
            float rotation;
        };

        struct CB_VS_Transform { XMFLOAT4X4 ViewProj; };
        struct CB_PS_Params { float Monochrome; XMFLOAT3 pad; };

    private:
        bool CreateShaders(ID3D11Device* device);
        bool CreateStates(ID3D11Device* device);
        bool EnsureVB(size_t spriteCapacity);
        bool EnsureIB(size_t spriteCapacity);
        void UpdateVSConstants();
        void UpdatePSConstants();

        // Build vertices for a group of draws sharing the same texture
        void BuildVertsForBucket(const std::vector<DrawCmd*>& bucket, std::vector<Vertex>& outVerts);

    private:
        // External
        ID3D11Device* m_device = nullptr; // not owned
        ID3D11DeviceContext* m_context = nullptr; // not owned

        // Pipeline
        ComPtr<ID3D11VertexShader> m_vs;
        ComPtr<ID3D11PixelShader>  m_ps;
        ComPtr<ID3D11InputLayout>  m_inputLayout;

        // Shared GPU resources
        ComPtr<ID3D11Buffer> m_cbVS;
        ComPtr<ID3D11Buffer> m_cbPS;
        ComPtr<ID3D11Buffer> m_vb;
        ComPtr<ID3D11Buffer> m_ib;

        // Fixed states
        ComPtr<ID3D11SamplerState> m_sampPoint;
        ComPtr<ID3D11SamplerState> m_sampLinear;
        ComPtr<ID3D11BlendState>   m_blendAlpha;
        ComPtr<ID3D11DepthStencilState> m_dssOff;
        ComPtr<ID3D11RasterizerState>   m_rsCullNone;

        // Batch data
        std::vector<DrawCmd> m_queue;
        XMFLOAT4X4           m_viewProj{};
        bool                 m_isDrawing = false;
        float                m_monochrome = 0.0f;
        bool                 m_pointSampling = true; // default: crisp pixels

        // Buffer capacities (number of sprites the buffers can hold without reallocate)
        size_t m_vbSpriteCap = 0;
        size_t m_ibSpriteCap = 0;
    };

}