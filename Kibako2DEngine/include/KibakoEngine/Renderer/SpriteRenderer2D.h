// Kibako2DEngine/include/KibakoEngine/Renderer/SpriteRenderer2D.h
#pragma once
#include <d3d11.h>
#include <wrl/client.h>
#include <DirectXMath.h>
#include "KibakoEngine/Renderer/Texture2D.h"
#include "KibakoEngine/Renderer/SpriteTypes.h"

namespace KibakoEngine {

    using Microsoft::WRL::ComPtr;
    using namespace DirectX;

    // =====================================================
    //  SpriteRenderer2D
    // =====================================================
    class SpriteRenderer2D
    {
    public:
        bool Init(ID3D11Device* device, ID3D11DeviceContext* context);
        void Shutdown();

        void Begin(const XMFLOAT4X4& viewProj);
        void End();

        void SetMonochrome(float amount) { m_monochrome = amount; }

        void DrawSprite(const Texture2D& tex,
            const RectF& dst,
            const RectF& src,
            const Color4& color,
            float rotation = 0.0f);

    private:
        Microsoft::WRL::ComPtr<ID3D11SamplerState> m_sampler;
        struct Vertex {
            XMFLOAT3 pos;
            XMFLOAT2 uv;
            XMFLOAT4 color;
        };

        struct CB_VS_Transform {
            XMFLOAT4X4 ViewProj;
        };

        struct CB_PS_Params {
            float Monochrome;
            XMFLOAT3 pad;
        };

        bool CreateShaders(ID3D11Device* device);
        bool CreateBuffers(ID3D11Device* device);
        void FlushDraw(const Texture2D& tex, int vertexCount);

    private:
        ID3D11Device* m_device = nullptr;
        ID3D11DeviceContext* m_context = nullptr;

        ComPtr<ID3D11VertexShader> m_vs;
        ComPtr<ID3D11PixelShader>  m_ps;
        ComPtr<ID3D11InputLayout>  m_inputLayout;

        ComPtr<ID3D11Buffer> m_vb;
        ComPtr<ID3D11Buffer> m_cbVS;
        ComPtr<ID3D11Buffer> m_cbPS;

        float m_monochrome = 0.0f;
        bool m_isDrawing = false;

        XMFLOAT4X4 m_viewProj{};
    };

}