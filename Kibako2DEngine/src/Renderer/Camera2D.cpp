// Kibako2DEngine/src/Renderer/Camera2D.cpp
#include "KibakoEngine/Renderer/Camera2D.h"
using namespace DirectX;

namespace KibakoEngine {

    void Camera2D::RebuildIfNeeded()
    {
        if (!m_dirty) return;
        m_dirty = false;

        // View: inverse of camera transform (2D: T * R * S)
        // World to camera: translate by -pos, rotate by -rot, scale by 1/zoom
        XMMATRIX T = XMMatrixTranslation(-m_posX, -m_posY, 0.0f);
        XMMATRIX R = XMMatrixRotationZ(-m_rot);
        XMMATRIX S = XMMatrixScaling(1.0f / m_zoom, 1.0f / m_zoom, 1.0f);
        m_view = S * R * T;

        // Projection: orthographic, Y-down pixel space
        // Use virtual size as logical units
        // OffCenterLH(left, right, bottom, top, near, far)
        float L = 0.0f;
        float Rr = static_cast<float>(m_vw);
        float B = static_cast<float>(m_vh);
        float Tt = 0.0f;
        m_proj = XMMatrixOrthographicOffCenterLH(L, Rr, B, Tt, 0.0f, 1.0f);

        // Transpose for HLSL (row-major CPU -> column-major HLSL)
        XMMATRIX vp = m_view * m_proj;
        XMStoreFloat4x4(&m_viewProjT, XMMatrixTranspose(vp));
    }

    DirectX::XMFLOAT4X4 Camera2D::GetViewProjT()
    {
        RebuildIfNeeded();
        return m_viewProjT;
    }

}