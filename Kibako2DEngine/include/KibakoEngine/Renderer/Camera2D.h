// Kibako2DEngine/include/KibakoEngine/Renderer/Camera2D.h
#pragma once
#include <DirectXMath.h>

namespace KibakoEngine {

    class Camera2D {
    public:
        // Virtual size = your logical 2D resolution (pixels)
        void SetVirtualSize(int w, int h) { m_vw = w; m_vh = h; m_dirty = true; }
        void SetViewportSize(int w, int h) { m_vpW = w; m_vpH = h; m_dirty = true; }

        // Camera transform
        void SetPosition(float x, float y) { m_posX = x; m_posY = y; m_dirty = true; }
        void SetZoom(float z) { m_zoom = z; m_dirty = true; }
        void SetRotation(float rads) { m_rot = rads; m_dirty = true; }

        // Accessors
        int VirtualW() const { return m_vw; }
        int VirtualH() const { return m_vh; }

        // Build and return ViewProj (transposed for HLSL)
        DirectX::XMFLOAT4X4 GetViewProjT();

    private:
        void RebuildIfNeeded();

    private:
        // Virtual resolution and viewport in pixels
        int m_vw = 1280, m_vh = 720;
        int m_vpW = 1280, m_vpH = 720;

        // Camera transform
        float m_posX = 0.0f;
        float m_posY = 0.0f;
        float m_zoom = 1.0f;     // 1.0 = no zoom
        float m_rot = 0.0f;     // radians

        // Cached matrices
        bool m_dirty = true;
        DirectX::XMMATRIX m_view = DirectX::XMMatrixIdentity();
        DirectX::XMMATRIX m_proj = DirectX::XMMatrixIdentity();
        DirectX::XMFLOAT4X4 m_viewProjT{}; // transposed for HLSL
    };

}