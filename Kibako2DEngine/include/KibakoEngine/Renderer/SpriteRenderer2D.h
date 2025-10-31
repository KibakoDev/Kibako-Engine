// =====================================================
// Kibako2DEngine/Renderer/SpriteRenderer2D.h
// Simple 2D sprite renderer (Direct3D11).
// Draws textured quads using an orthographic camera.
// =====================================================

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
    // SpriteRenderer2D
    // =====================================================
    // Handles simple textured 2D rendering.
    // Each DrawSprite() call sends vertices immediately.
    // Used for UI, debug visuals, or simple 2D games.
    // =====================================================
    class SpriteRenderer2D {
    public:
        // Initializes shaders, buffers, and default GPU states
        bool Init(ID3D11Device* device, ID3D11DeviceContext* context);

        // Frees all GPU resources
        void Shutdown();

        // Begins a new sprite batch for this frame.
        // viewProj = camera transform from world to clip space.
        void Begin(const XMFLOAT4X4& viewProj);

        // Ends the current batch.
        void End();

        // Controls grayscale rendering.
        // 0.0 = full color, 1.0 = full monochrome.
        void SetMonochrome(float amount) { m_monochrome = amount; }

        // Draws a textured quad.
        // tex: texture to draw
        // dst: screen/world position (x, y, width, height)
        // src: texture UVs (0..1)
        // color: vertex tint (RGBA)
        // rotation: rotation around the quad center (in radians)
        void DrawSprite(const Texture2D& tex,
            const RectF& dst,
            const RectF& src,
            const Color4& color,
            float rotation = 0.0f);

    private:
        // =====================================================
        // Internal Structures
        // =====================================================

        // Vertex format used by the shader
        struct Vertex {
            XMFLOAT3 pos;   // position in world space
            XMFLOAT2 uv;    // texture coordinates
            XMFLOAT4 color; // per-vertex color (tint)
        };

        // Constant buffer: camera transform (vertex shader)
        struct CB_VS_Transform {
            XMFLOAT4X4 ViewProj;
        };

        // Constant buffer: parameters for pixel shader
        struct CB_PS_Params {
            float     Monochrome; // grayscale intensity
            XMFLOAT3  pad;        // padding for 16-byte alignment
        };

        // =====================================================
        // Internal Helpers
        // =====================================================

        // Creates and compiles vertex/pixel shaders
        bool CreateShaders(ID3D11Device* device);

        // Creates vertex and constant buffers
        bool CreateBuffers(ID3D11Device* device);

        // Creates fixed GPU states (sampler, blending, etc.)
        bool CreateStates(ID3D11Device* device);

        // Submits vertices to GPU and issues draw call
        void FlushDraw(const Texture2D& tex, int vertexCount);

    private:
        // =====================================================
        // Member Variables
        // =====================================================

        // External references (not owned)
        ID3D11Device* m_device = nullptr;
        ID3D11DeviceContext* m_context = nullptr;

        // Core D3D pipeline objects
        ComPtr<ID3D11VertexShader> m_vs;
        ComPtr<ID3D11PixelShader>  m_ps;
        ComPtr<ID3D11InputLayout>  m_inputLayout;

        // GPU buffers
        ComPtr<ID3D11Buffer> m_vb;   // vertex buffer (6 vertices per quad)
        ComPtr<ID3D11Buffer> m_cbVS; // camera constants
        ComPtr<ID3D11Buffer> m_cbPS; // pixel constants (monochrome)

        // Fixed states
        ComPtr<ID3D11SamplerState> m_sampler; // texture sampling mode
        ComPtr<ID3D11BlendState>   m_blend;   // alpha blending

        // Frame state
        float       m_monochrome = 0.0f; // grayscale level
        bool        m_isDrawing = false;
        XMFLOAT4X4  m_viewProj{};        // cached camera matrix
    };

}