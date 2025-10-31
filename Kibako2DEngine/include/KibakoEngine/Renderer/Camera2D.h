// =====================================================
// Kibako2DEngine/Renderer/Camera2D.h
// 2D orthographic camera supporting zoom, rotation, and pan.
// =====================================================

#pragma once
#include <DirectXMath.h>

namespace KibakoEngine {

    class Camera2D {
    public:
        // Virtual size = logical resolution (in pixels)
        void SetVirtualSize(int w, int h) { m_vw = w; m_vh = h; m_dirty = true; }
        void SetViewportSize(int w, int h) { m_vpW = w; m_vpH = h; m_dirty = true; }

        // Camera transform controls
        void SetPosition(float x, float y) { m_posX = x; m_posY = y; m_dirty = true; }
        void SetZoom(float z) { m_zoom = z; m_dirty = true; }
        void SetRotation(float rads) { m_rot = rads; m_dirty = true; }

        // Getters
        int VirtualW() const { return m_vw; }
        int VirtualH() const { return m_vh; }

        // Returns the final ViewProjection matrix (transposed for HLSL)
        DirectX::XMFLOAT4X4 GetViewProjT();

        // Accessors for transform
        float X() const { return m_posX; }
        float Y() const { return m_posY; }
        float Zoom() const { return m_zoom; }
        float Rotation() const { return m_rot; }

        // Movement helpers
        void Move(float dx, float dy) { m_posX += dx; m_posY += dy; m_dirty = true; }
        void AddZoom(float dz) {
            m_zoom += dz;
            if (m_zoom < 0.05f) m_zoom = 0.05f;
            if (m_zoom > 8.0f)  m_zoom = 8.0f;
            m_dirty = true;
        }
        void AddRotation(float dr) { m_rot += dr; m_dirty = true; }
        void Reset() { m_posX = 0.0f; m_posY = 0.0f; m_zoom = 1.0f; m_rot = 0.0f; m_dirty = true; }

    private:
        void RebuildIfNeeded();

    private:
        // Virtual resolution and viewport (in pixels)
        int m_vw = 1280, m_vh = 720;
        int m_vpW = 1280, m_vpH = 720;

        // Transform state
        float m_posX = 0.0f;
        float m_posY = 0.0f;
        float m_zoom = 1.0f;
        float m_rot = 0.0f;

        // Cached matrices
        bool m_dirty = true;
        DirectX::XMMATRIX   m_view = DirectX::XMMatrixIdentity();
        DirectX::XMMATRIX   m_proj = DirectX::XMMatrixIdentity();
        DirectX::XMFLOAT4X4 m_viewProjT{};
    };

}