#pragma once

#include <d3d11.h>
#include <wrl/client.h>

#include <DirectXMath.h>

#include <vector>

#include "KibakoEngine/Renderer/SpriteTypes.h"
#include "KibakoEngine/Renderer/Texture2D.h"

namespace KibakoEngine {

    class SpriteBatch2D {
    public:
        bool Init(ID3D11Device* device, ID3D11DeviceContext* context);
        void Shutdown();

        void Begin(const DirectX::XMFLOAT4X4& viewProjT);
        void End();

        void SetPixelSnap(bool enabled) { m_pixelSnap = enabled; }

        void Push(const Texture2D& texture,
                  const RectF& dst,
                  const RectF& src,
                  const Color4& color,
                  float rotation = 0.0f,
                  int layer = 0);

    private:
        struct Vertex {
            DirectX::XMFLOAT3 position;
            DirectX::XMFLOAT2 uv;
            DirectX::XMFLOAT4 color;
        };

        struct DrawCommand {
            const Texture2D* texture = nullptr;
            RectF  dst;
            RectF  src;
            Color4 color;
            float  rotation = 0.0f;
            int    layer = 0;
        };

        struct CBVS {
            DirectX::XMFLOAT4X4 viewProjT;
        };

        bool CreateShaders(ID3D11Device* device);
        bool CreateStates(ID3D11Device* device);
        bool EnsureVertexCapacity(size_t spriteCount);
        bool EnsureIndexCapacity(size_t spriteCount);
        void UpdateVSConstants();
        void BuildVertices(std::vector<Vertex>& outVertices) const;

        ID3D11Device*        m_device = nullptr;
        ID3D11DeviceContext* m_context = nullptr;

        Microsoft::WRL::ComPtr<ID3D11VertexShader> m_vs;
        Microsoft::WRL::ComPtr<ID3D11PixelShader>  m_ps;
        Microsoft::WRL::ComPtr<ID3D11InputLayout>  m_inputLayout;
        Microsoft::WRL::ComPtr<ID3D11Buffer>       m_vertexBuffer;
        Microsoft::WRL::ComPtr<ID3D11Buffer>       m_indexBuffer;
        Microsoft::WRL::ComPtr<ID3D11Buffer>       m_cbVS;
        Microsoft::WRL::ComPtr<ID3D11SamplerState> m_samplerPoint;
        Microsoft::WRL::ComPtr<ID3D11BlendState>   m_blendAlpha;
        Microsoft::WRL::ComPtr<ID3D11DepthStencilState> m_depthDisabled;
        Microsoft::WRL::ComPtr<ID3D11RasterizerState>   m_rasterCullNone;

        std::vector<DrawCommand> m_commands;
        std::vector<uint32_t>    m_indexScratch;

        DirectX::XMFLOAT4X4 m_viewProjT{};
        size_t m_vertexCapacitySprites = 0;
        size_t m_indexCapacitySprites = 0;
        bool   m_pixelSnap = true;
        bool   m_isDrawing = false;
    };

} // namespace KibakoEngine

