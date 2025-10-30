// Kibako2DEngine/include/KibakoEngine/Renderer/RendererD3D11.h
#pragma once
#include <windows.h>
#include <d3d11.h>
#include <dxgi.h>
#include <wrl/client.h>
#include <DirectXMath.h>
#include "KibakoEngine/Renderer/Camera2D.h"

namespace KibakoEngine {

    using Microsoft::WRL::ComPtr;

    class RendererD3D11 {
    public:
        bool Init(HWND hwnd, int width, int height);
        void BeginFrame();
        void EndFrame();
        void OnResize(int newWidth, int newHeight);
        void Shutdown();

        ID3D11Device* GetDevice()  const { return m_device.Get(); }
        ID3D11DeviceContext* GetContext() const { return m_context.Get(); }

        Camera2D& Camera() { return m_camera; }

    private:
        bool CreateRTV();
        void DestroyRTV();

        bool InitTrianglePipeline();
        void DrawTriangle();
        void DestroyTriangle();

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

        // Triangle pipeline
        ComPtr<ID3D11VertexShader> m_vs;
        ComPtr<ID3D11PixelShader>  m_ps;
        ComPtr<ID3D11InputLayout>  m_inputLayout;
        ComPtr<ID3D11Buffer>       m_vb;
        UINT                       m_vbStride = 0;
        UINT                       m_vbOffset = 0;

        // Camera and constant buffer
        Camera2D m_camera;
        struct CB_VS_Camera { DirectX::XMFLOAT4X4 ViewProj; };
        ComPtr<ID3D11Buffer> m_cbCamera;
    };

}
