// Kibako2DEngine/include/KibakoEngine/Renderer/RendererD3D11.h
#pragma once

#include <windows.h>
#include <d3d11.h>
#include <dxgi.h>
#include <cstdint>

namespace KibakoEngine
{
    class RendererD3D11
    {
    public:
        bool Init(HWND hwnd, int width, int height);
        void BeginFrame();
        void EndFrame();
        void OnResize(int newWidth, int newHeight);
        void Shutdown();

        inline ID3D11Device* GetDevice() const { return m_device; }
        inline ID3D11DeviceContext* GetContext() const { return m_context; }

    private:
        bool CreateRTV();
        void DestroyRTV();

        ID3D11Device* m_device = nullptr;
        ID3D11DeviceContext* m_context = nullptr;
        IDXGISwapChain* m_swapChain = nullptr;
        ID3D11RenderTargetView* m_rtv = nullptr;

        int m_width = 0;
        int m_height = 0;
        HWND m_hwnd = nullptr;
    };
}