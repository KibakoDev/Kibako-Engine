#include "KibakoEngine/Renderer/Texture2D.h"

#include "KibakoEngine/Core/Debug.h"
#include "KibakoEngine/Core/Log.h"
#include "KibakoEngine/Core/Profiler.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

namespace KibakoEngine {

    namespace
    {
        constexpr const char* kLogChannel = "Texture";
    }

    void Texture2D::Reset()
    {
        m_srv.Reset();
        m_texture.Reset();
        m_width = 0;
        m_height = 0;
    }

    bool Texture2D::CreateSolidColor(ID3D11Device* device, std::uint32_t width, std::uint32_t height, std::uint32_t rgba)
    {
        KBK_ASSERT(device != nullptr, "Texture2D::CreateSolidColor requires a valid device");
        KBK_ASSERT(width > 0 && height > 0, "Texture2D::CreateSolidColor requires non-zero size");

        Reset();

        const D3D11_TEXTURE2D_DESC desc{
            width,
            height,
            1,
            1,
            DXGI_FORMAT_R8G8B8A8_UNORM,
            { 1, 0 },
            D3D11_USAGE_IMMUTABLE,
            D3D11_BIND_SHADER_RESOURCE,
            0,
            0
        };

        const std::uint32_t pixel = rgba;

        const D3D11_SUBRESOURCE_DATA data{ &pixel, width * sizeof(std::uint32_t), 0 };

        Microsoft::WRL::ComPtr<ID3D11Texture2D> texture;
        HRESULT hr = device->CreateTexture2D(&desc, &data, texture.GetAddressOf());
        if (FAILED(hr)) {
            KbkError(kLogChannel, "CreateTexture2D (solid) failed: 0x%08X", static_cast<unsigned>(hr));
            return false;
        }

        Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srv;
        hr = device->CreateShaderResourceView(texture.Get(), nullptr, srv.GetAddressOf());
        if (FAILED(hr)) {
            KbkError(kLogChannel, "CreateShaderResourceView (solid) failed: 0x%08X", static_cast<unsigned>(hr));
            return false;
        }

        m_texture = texture;
        m_srv = srv;
        m_width = static_cast<int>(width);
        m_height = static_cast<int>(height);
        return true;
    }

    bool Texture2D::LoadFromFile(ID3D11Device* device, const std::string& path, bool srgb)
    {
        KBK_PROFILE_SCOPE("TextureLoad");

        KBK_ASSERT(device != nullptr, "Texture2D::LoadFromFile requires a valid device");
        Reset();

        stbi_set_flip_vertically_on_load(false);
        int width = 0;
        int height = 0;
        int comp = 0;
        stbi_uc* pixels = stbi_load(path.c_str(), &width, &height, &comp, 4);
        if (!pixels) {
            KbkError(kLogChannel, "Failed to load %s", path.c_str());
            return false;
        }

        const DXGI_FORMAT format = srgb ? DXGI_FORMAT_R8G8B8A8_UNORM_SRGB : DXGI_FORMAT_R8G8B8A8_UNORM;

        D3D11_TEXTURE2D_DESC desc{};
        desc.Width = static_cast<UINT>(width);
        desc.Height = static_cast<UINT>(height);
        desc.MipLevels = 1;
        desc.ArraySize = 1;
        desc.Format = format;
        desc.SampleDesc.Count = 1;
        desc.Usage = D3D11_USAGE_IMMUTABLE;
        desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

        D3D11_SUBRESOURCE_DATA data{};
        data.pSysMem = pixels;
        data.SysMemPitch = static_cast<UINT>(width * 4);

        Microsoft::WRL::ComPtr<ID3D11Texture2D> texture;
        HRESULT hr = device->CreateTexture2D(&desc, &data, texture.GetAddressOf());
        stbi_image_free(pixels);
        if (FAILED(hr)) {
            KbkError(kLogChannel, "CreateTexture2D failed: 0x%08X", static_cast<unsigned>(hr));
            return false;
        }

        Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srv;
        hr = device->CreateShaderResourceView(texture.Get(), nullptr, srv.GetAddressOf());
        if (FAILED(hr)) {
            KbkError(kLogChannel, "CreateShaderResourceView failed: 0x%08X", static_cast<unsigned>(hr));
            return false;
        }

        m_texture = texture;
        m_srv = srv;
        m_width = width;
        m_height = height;
        KbkLog(kLogChannel, "Loaded %s (%dx%d)", path.c_str(), m_width, m_height);
        return true;
    }

} // namespace KibakoEngine

