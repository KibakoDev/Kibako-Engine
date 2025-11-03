// =====================================================
// Kibako2DEngine/Renderer/RendererD3D11.h
// D3D11 device/swapchain/RTV + camera + sprite batch glue.
// No game logic here.
// =====================================================

#pragma once
#include <windows.h>
#include <d3d11.h>
#include <dxgi.h>
#include <wrl/client.h>
#include <DirectXMath.h>

#include "KibakoEngine/Renderer/Camera2D.h"
#include "KibakoEngine/Renderer/SpriteBatch2D.h"

namespace KibakoEngine {

    using Microsoft::WRL::ComPtr;

    class RendererD3D11 {
    public:
        // Device / swapchain / camera / sprite systems
        bool Init(HWND hwnd, int width, int height);
        void Shutdown();

        // Per-frame hooks (clear, present)
        void BeginFrame();
        void EndFrame();

        // Resize window/swapchain/viewport/camera
        void OnResize(int newWidth, int newHeight);

        // Accessors
        ID3D11Device* GetDevice()  const { return m_device.Get(); }
        ID3D11DeviceContext* GetContext() const { return m_context.Get(); }
        Camera2D& Camera() { return m_camera; }
        SpriteBatch2D& Batch() { return m_spriteBatch; }

    private:
        // RTV lifecycle
        bool CreateRTV();
        void DestroyRTV();

        // Camera constant buffer
        bool CreateConstantBuffers();
        void UpdateCameraCB();

    private:
        // Core D3D
        ComPtr<ID3D11Device>           m_device;
        ComPtr<ID3D11DeviceContext>    m_context;
        ComPtr<IDXGISwapChain>         m_swapChain;
        ComPtr<ID3D11RenderTargetView> m_rtv;

        int  m_width = 0;
        int  m_height = 0;
        HWND m_hwnd = nullptr;

        // Camera + VS constant buffer
        Camera2D m_camera;
        struct CB_VS_Camera { DirectX::XMFLOAT4X4 ViewProj; };
        ComPtr<ID3D11Buffer> m_cbCamera;

        // 2D sprite batching pipeline
        SpriteBatch2D m_spriteBatch;
    };
}