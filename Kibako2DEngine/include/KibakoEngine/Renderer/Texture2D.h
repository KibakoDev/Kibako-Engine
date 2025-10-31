// =====================================================
// Kibako2DEngine/Renderer/Texture2D.h
// Simple wrapper for 2D textures loaded with stb_image.
// =====================================================
#pragma once
#include <string>
#include <wrl/client.h>
#include <d3d11.h>

namespace KibakoEngine {

    class Texture2D {
    public:
        // Loads an image from disk and creates a GPU texture
        bool LoadFromFile(ID3D11Device* device, const std::string& path, bool forceRGBA = true);

        // Accessors
        ID3D11ShaderResourceView* GetSRV() const { return m_srv.Get(); }
        int  Width()  const { return m_w; }
        int  Height() const { return m_h; }

        // Frees GPU memory
        void Reset();

    private:
        Microsoft::WRL::ComPtr<ID3D11Texture2D>          m_tex;
        Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_srv;
        int m_w = 0;
        int m_h = 0;
    };

}