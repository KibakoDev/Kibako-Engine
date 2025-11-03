// =====================================================
// Kibako2DEngine/Renderer/Camera2D.cpp
// Builds the 2D orthographic view-projection matrix.
// =====================================================

#include "KibakoEngine/Renderer/Camera2D.h"
using namespace DirectX;

namespace KibakoEngine {

    // Recompute View, Proj, and ViewProj^T if dirty
    void Camera2D::RebuildMatrices()
    {
        // View: rotate then translate (no zoom/scale)
        XMMATRIX V =
            XMMatrixRotationZ(-m_rot) *
            XMMatrixTranslation(-m_pos.x, -m_pos.y, 0.0f);

        // Projection: map virtual space (0..W, 0..H) to clip space
        // Left-handed, origin at top-left, Y down.
        float L = 0.0f, T = 0.0f;
        float R = static_cast<float>(m_virtualW);
        float B = static_cast<float>(m_virtualH);
        XMMATRIX P = XMMatrixOrthographicOffCenterLH(L, R, B, T, 0.0f, 1.0f);

        XMMATRIX VP = V * P;

        XMStoreFloat4x4(&m_view, V);
        XMStoreFloat4x4(&m_proj, P);

        // Transpose for HLSL constant buffer
        XMMATRIX VP_T = XMMatrixTranspose(VP);
        XMStoreFloat4x4(&m_viewProjT, VP_T);

        m_dirty = false;
    }

    XMFLOAT4X4 Camera2D::GetViewProjT()
    {
        if (m_dirty) RebuildMatrices();
        return m_viewProjT;
    }

}