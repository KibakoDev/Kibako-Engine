// Kibako2DEngine/include/KibakoEngine/Renderer/SpriteBatch2D.h
#pragma once
#include <d3d11.h>
#include <wrl/client.h>
#include <DirectXMath.h>
#include <vector>
#include <unordered_map>
#include <cstdint>

#include "KibakoEngine/Renderer/Texture2D.h"
#include "KibakoEngine/Renderer/SpriteTypes.h"

namespace KibakoEngine {

    using Microsoft::WRL::ComPtr;

    // Batched 2D sprite renderer:
    // - Push() des sprites pendant la frame
    // - End() regroupe par texture et fait un DrawIndexed par texture
    class SpriteBatch2D {
    public:
        bool Init(ID3D11Device* device, ID3D11DeviceContext* context);
        void Shutdown();

        // Délimite une frame de sprites
        void Begin(const DirectX::XMFLOAT4X4& viewProj);
        void End();

        // Paramètres visuels globaux
        void SetMonochrome(float amount) { m_monochrome = amount; }     // 0 = couleur, 1 = N&B
        void SetPointSampling(bool enable) { m_pointSampling = enable; }  // point vs linear
        void SetPixelSnap(bool enable) { m_pixelSnap = enable; }      // arrondit aux pixels

        // Ajoute un sprite dans le batch
        // dst: x,y,w,h en pixels monde
        // src: rect UV [0..1]
        // color: teinte
        // rotation: radians (centre du dst)
        // layer: ordre de dessin à texture égale (plus petit = derrière)
        void Push(const Texture2D& tex,
            const RectF& dst,
            const RectF& src,
            const Color4& color,
            float rotation = 0.0f,
            int   layer = 0);

    private:
        struct Vertex {
            DirectX::XMFLOAT3 pos;
            DirectX::XMFLOAT2 uv;
            DirectX::XMFLOAT4 color;
        };

        struct CB_VS_Transform { DirectX::XMFLOAT4X4 ViewProj; };
        struct CB_PS_Params { float Monochrome; DirectX::XMFLOAT3 _pad; }; // 16 bytes

        struct DrawCmd {
            const Texture2D* tex = nullptr;
            RectF  dst{};
            RectF  src{};
            Color4 color{ 1,1,1,1 };
            float  rotation = 0.0f;
            int    layer = 0;
        };

        bool CreateShaders(ID3D11Device* device);
        bool CreateStates(ID3D11Device* device);
        bool EnsureVB(size_t spriteCapacity); // capacité en sprites
        bool EnsureIB(size_t spriteCapacity);

        void UpdateVSConstants();
        void UpdatePSConstants();
        void BuildVertsForBucket(const std::vector<DrawCmd*>& bucket,
            std::vector<Vertex>& outVerts);

    private:
        // External D3D (non-owning)
        ID3D11Device* m_device = nullptr;
        ID3D11DeviceContext* m_context = nullptr;

        // Pipeline
        ComPtr<ID3D11VertexShader> m_vs;
        ComPtr<ID3D11PixelShader>  m_ps;
        ComPtr<ID3D11InputLayout>  m_inputLayout;

        // Buffers partagés (taille variable)
        ComPtr<ID3D11Buffer> m_vb;      // N sprites * 4 verts
        ComPtr<ID3D11Buffer> m_ib;      // N sprites * 6 indices
        size_t m_vbSpriteCap = 0;       // capacité en sprites
        size_t m_ibSpriteCap = 0;

        // Constant buffers
        ComPtr<ID3D11Buffer> m_cbVS;
        ComPtr<ID3D11Buffer> m_cbPS;

        // Fixed states
        ComPtr<ID3D11SamplerState>      m_sampPoint;
        ComPtr<ID3D11SamplerState>      m_sampLinear;
        ComPtr<ID3D11BlendState>        m_blendAlpha;
        ComPtr<ID3D11DepthStencilState> m_dssOff;
        ComPtr<ID3D11RasterizerState>   m_rsCullNone;

        // Contexte frame
        std::vector<DrawCmd>   m_queue;
        DirectX::XMFLOAT4X4    m_viewProj{};
        float                  m_monochrome = 0.0f;
        bool                   m_pointSampling = true;   // par défaut: pixels nets
        bool                   m_pixelSnap = true;   // par défaut: snap aux pixels
        bool                   m_isDrawing = false;
    };

}