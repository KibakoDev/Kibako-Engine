// Kibako2DEngine/include/KibakoEngine/Renderer/RendererD3D11.h
#pragma once
#include <windows.h>
#include <d3d11.h>
#include <dxgi.h>

namespace KibakoEngine {

    class RendererD3D11 {
    public:
        bool Init(HWND hwnd, int width, int height);
        void BeginFrame();
        void EndFrame();
        void OnResize(int newWidth, int newHeight);
        void Shutdown();

        ID3D11Device* GetDevice()  const { return m_device; }
        ID3D11DeviceContext* GetContext() const { return m_context; }

    private:
        bool CreateRTV();
        void DestroyRTV();

        // === Nouveau ===
        bool InitTrianglePipeline();
        void DrawTriangle();
        void DestroyTriangle();

        // === D3D objects ===
        ID3D11Device* m_device = nullptr;
        ID3D11DeviceContext* m_context = nullptr;
        IDXGISwapChain* m_swapChain = nullptr;
        ID3D11RenderTargetView* m_rtv = nullptr;

        int  m_width = 0, m_height = 0;
        HWND m_hwnd = nullptr;

        // === Nouveau : pipeline du triangle ===
        ID3D11VertexShader* m_vs = nullptr;
        ID3D11PixelShader* m_ps = nullptr;
        ID3D11InputLayout* m_inputLayout = nullptr;
        ID3D11Buffer* m_vb = nullptr;
        UINT                m_vbStride = 0;
        UINT                m_vbOffset = 0;
    };

}