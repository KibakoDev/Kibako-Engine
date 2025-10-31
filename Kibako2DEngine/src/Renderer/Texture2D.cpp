// Kibako2DEngine/src/Renderer/Texture2D.cpp
#define STB_IMAGE_IMPLEMENTATION
#include "KibakoEngine/Renderer/Texture2D.h"
#include <wrl/client.h>
#include <d3d11.h>
#include <vector>
#include <cstring>

// stb_image (CPU decode)
#include "stb_image.h"

namespace KibakoEngine {

    void Texture2D::Reset() {
        m_srv.Reset();
        m_tex.Reset();
        m_w = m_h = 0;
    }

    bool Texture2D::LoadFromFile(ID3D11Device* device, const std::string& path, bool forceRGBA)
    {
        Reset();

        int w = 0, h = 0, comp = 0;
        stbi_uc* pixels = stbi_load(path.c_str(), &w, &h, &comp, forceRGBA ? 4 : 0);
        if (!pixels) {
            // stbi_failure_reason() existe si tu veux logguer le détail
            return false;
        }

        const int channels = forceRGBA ? 4 : comp;
        const DXGI_FORMAT fmt = (channels == 4) ? DXGI_FORMAT_R8G8B8A8_UNORM
            : DXGI_FORMAT_R8_UNORM;

        // Describe the GPU texture
        D3D11_TEXTURE2D_DESC td{};
        td.Width = w;
        td.Height = h;
        td.MipLevels = 1;                // single mip for now
        td.ArraySize = 1;
        td.Format = fmt;
        td.SampleDesc.Count = 1;         // no MSAA
        td.Usage = D3D11_USAGE_IMMUTABLE;// data provided at creation
        td.BindFlags = D3D11_BIND_SHADER_RESOURCE;

        D3D11_SUBRESOURCE_DATA srd{};
        srd.pSysMem = pixels;
        srd.SysMemPitch = w * channels;  // bytes per row

        Microsoft::WRL::ComPtr<ID3D11Texture2D> tex;
        HRESULT hr = device->CreateTexture2D(&td, &srd, tex.ReleaseAndGetAddressOf());

        // CPU memory no longer needed
        stbi_image_free(pixels);

        if (FAILED(hr)) {
            return false;
        }

        // Create SRV (shader view)
        D3D11_SHADER_RESOURCE_VIEW_DESC srvd{};
        srvd.Format = td.Format;
        srvd.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
        srvd.Texture2D.MipLevels = 1;
        
        Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srv;
        hr = device->CreateShaderResourceView(tex.Get(), &srvd, srv.ReleaseAndGetAddressOf());
        if (FAILED(hr)) {
            return false;
        }

        // Store
        m_tex = tex;
        m_srv = srv;
        m_w = w; m_h = h;
        return true;
    }

}