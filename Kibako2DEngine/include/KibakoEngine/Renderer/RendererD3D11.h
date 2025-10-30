// Kibako2DEngine/include/KibakoEngine/Renderer/RendererD3D11.h
#pragma once
#include <windows.h>
#include <d3d11.h>
#include <dxgi.h>
#include <wrl/client.h>

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

    private:
        // Back buffer RTV helpers
        bool CreateRTV();
        void DestroyRTV();

        // Minimal pipeline to draw one triangle
        bool InitTrianglePipeline();
        void DrawTriangle();
        void DestroyTriangle();

    private:
        // Core D3D objects (managed via ComPtr)
        ComPtr<ID3D11Device>           m_device;
        ComPtr<ID3D11DeviceContext>    m_context;
        ComPtr<IDXGISwapChain>         m_swapChain;
        ComPtr<ID3D11RenderTargetView> m_rtv;

        // Window and size
        int  m_width = 0;
        int  m_height = 0;
        HWND m_hwnd = nullptr;

        // Triangle pipeline resources
        ComPtr<ID3D11VertexShader> m_vs;
        ComPtr<ID3D11PixelShader>  m_ps;
        ComPtr<ID3D11InputLayout>  m_inputLayout;
        ComPtr<ID3D11Buffer>       m_vb;
        UINT                       m_vbStride = 0;
        UINT                       m_vbOffset = 0;
    };

}