// Direct3D 11 renderer and sprite batch owner
#pragma once

#include <d3d11.h>
#include <dxgi.h>
#include <wrl/client.h>

#include <cstdint>

#include "KibakoEngine/Renderer/Camera2D.h"
#include "KibakoEngine/Renderer/SpriteBatch2D.h"

struct HWND__;
using HWND = HWND__*;

namespace KibakoEngine {

    class RendererD3D11 {
    public:
        bool Init(HWND hwnd, uint32_t width, uint32_t height);
        void Shutdown();

        void BeginFrame(const float clearColor[4]);
        void EndFrame(bool waitForVSync);
        void OnResize(uint32_t width, uint32_t height);

        [[nodiscard]] ID3D11Device* GetDevice() const { return m_device.Get(); }
        [[nodiscard]] ID3D11DeviceContext* GetImmediateContext() const { return m_context.Get(); }
        [[nodiscard]] Camera2D& Camera() { return m_camera; }
        [[nodiscard]] SpriteBatch2D& Batch() { return m_batch; }
        [[nodiscard]] uint32_t Width() const { return m_width; }
        [[nodiscard]] uint32_t Height() const { return m_height; }

    private:
        bool CreateSwapChain(HWND hwnd, uint32_t width, uint32_t height);
        bool CreateRenderTargets(uint32_t width, uint32_t height);

        Microsoft::WRL::ComPtr<ID3D11Device> m_device;
        Microsoft::WRL::ComPtr<ID3D11DeviceContext> m_context;
        Microsoft::WRL::ComPtr<IDXGISwapChain> m_swapChain;
        Microsoft::WRL::ComPtr<ID3D11RenderTargetView> m_rtv;
        D3D_FEATURE_LEVEL m_featureLevel = D3D_FEATURE_LEVEL_11_0;

        Camera2D     m_camera;
        SpriteBatch2D m_batch;
        uint32_t     m_width = 0;
        uint32_t     m_height = 0;
    };

} // namespace KibakoEngine

