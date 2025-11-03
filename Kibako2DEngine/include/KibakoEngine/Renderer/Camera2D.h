// =====================================================
// Kibako2DEngine/Renderer/Camera2D.h
// 2D orthographic camera, rotation, and pan.
// =====================================================

#pragma once
#include <DirectXMath.h>

namespace KibakoEngine {

    using namespace DirectX;

    // Very small 2D camera: position + rotation only. No zoom.
    class Camera2D {
    public:
        // Set virtual (world) size and output viewport size (in pixels)
        void SetVirtualSize(int w, int h) { m_virtualW = w; m_virtualH = h; m_dirty = true; }
        void SetViewportSize(int w, int h) { m_viewW = w; m_viewH = h; m_dirty = true; }

        // Position / rotation
        void SetPosition(float x, float y) { m_pos = { x, y }; m_dirty = true; }
        void Move(float dx, float dy) { m_pos.x += dx; m_pos.y += dy; m_dirty = true; }

        void SetRotation(float rads) { m_rot = rads; m_dirty = true; }
        void AddRotation(float dr) { m_rot += dr; m_dirty = true; }

        // Reset to default (no rotation, origin)
        void Reset() { m_pos = { 0,0 }; m_rot = 0.0f; m_dirty = true; }

        // Build and return View*Proj (transposed for HLSL cbuffer layout)
        DirectX::XMFLOAT4X4 GetViewProjT();

        // Read-only helpers
        float Rotation() const { return m_rot; }
        DirectX::XMFLOAT2 Position() const { return m_pos; }
        int VirtualW() const { return m_virtualW; }
        int VirtualH() const { return m_virtualH; }

    private:
        void RebuildMatrices();

    private:
        // Camera state (no zoom kept)
        DirectX::XMFLOAT2 m_pos{ 0,0 };
        float m_rot = 0.0f;

        int   m_virtualW = 1;
        int   m_virtualH = 1;
        int   m_viewW = 1;
        int   m_viewH = 1;

        bool  m_dirty = true;

        // Cached matrices
        DirectX::XMFLOAT4X4 m_view{};
        DirectX::XMFLOAT4X4 m_proj{};
        DirectX::XMFLOAT4X4 m_viewProjT{};
    };

}