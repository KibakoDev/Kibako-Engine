// Orthographic camera logic
#include "KibakoEngine/Renderer/Camera2D.h"

using namespace DirectX;

namespace KibakoEngine {

    void Camera2D::SetViewport(float width, float height)
    {
        if (width <= 0.0f) width = 1.0f;
        if (height <= 0.0f) height = 1.0f;
        if (width != m_viewWidth || height != m_viewHeight) {
            m_viewWidth = width;
            m_viewHeight = height;
            m_dirty = true;
            UpdateMatrix();
        }
    }

    void Camera2D::SetPosition(float x, float y)
    {
        if (x != m_positionX || y != m_positionY) {
            m_positionX = x;
            m_positionY = y;
            m_dirty = true;
            UpdateMatrix();
        }
    }

    void Camera2D::SetRotation(float radians)
    {
        if (radians != m_rotation) {
            m_rotation = radians;
            m_dirty = true;
            UpdateMatrix();
        }
    }

    void Camera2D::UpdateMatrix()
    {
        if (!m_dirty)
            return;
        m_dirty = false;

        const XMMATRIX proj = XMMatrixOrthographicOffCenterLH(0.0f, m_viewWidth, m_viewHeight, 0.0f, -1.0f, 1.0f);
        const XMMATRIX translate = XMMatrixTranslation(-m_positionX, -m_positionY, 0.0f);
        const XMMATRIX rotate = XMMatrixRotationZ(-m_rotation);
        const XMMATRIX view = rotate * translate;
        const XMMATRIX vp = view * proj;

        XMStoreFloat4x4(&m_viewProj, vp);
        const XMMATRIX vpT = XMMatrixTranspose(vp);
        XMStoreFloat4x4(&m_viewProjT, vpT);
    }

} // namespace KibakoEngine

