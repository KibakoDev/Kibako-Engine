// =====================================================
// Kibako2DEngine/Renderer/Camera2D.cpp
// Builds the 2D orthographic view-projection matrix.
// =====================================================

#include "KibakoEngine/Renderer/Camera2D.h"
using namespace DirectX;

namespace KibakoEngine {

    void Camera2D::RebuildIfNeeded() {
        if (!m_dirty) return;
        m_dirty = false;

        // Build view matrix (inverse of camera transform)
        XMMATRIX T = XMMatrixTranslation(-m_posX, -m_posY, 0.0f);
        XMMATRIX R = XMMatrixRotationZ(-m_rot);
        XMMATRIX S = XMMatrixScaling(1.0f / m_zoom, 1.0f / m_zoom, 1.0f);
        m_view = S * R * T;

        // Build orthographic projection (Y down)
        float L = 0.0f;
        float Rr = static_cast<float>(m_vw);
        float B = static_cast<float>(m_vh);
        float Tt = 0.0f;
        m_proj = XMMatrixOrthographicOffCenterLH(L, Rr, B, Tt, 0.0f, 1.0f);

        // Store transposed ViewProjection for HLSL
        XMMATRIX vp = m_view * m_proj;
        XMStoreFloat4x4(&m_viewProjT, XMMatrixTranspose(vp));
    }

    XMFLOAT4X4 Camera2D::GetViewProjT() {
        RebuildIfNeeded();
        return m_viewProjT;
    }

}