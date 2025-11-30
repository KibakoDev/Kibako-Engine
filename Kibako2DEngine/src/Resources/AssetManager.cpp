// Asset loading and caching
#include "KibakoEngine/Resources/AssetManager.h"

#include "KibakoEngine/Core/Debug.h"
#include "KibakoEngine/Core/Log.h"

namespace KibakoEngine {

    namespace
    {
        constexpr const char* kLogChannel = "Assets";
    }

    void AssetManager::Init(ID3D11Device* device)
    {
        KBK_ASSERT(device != nullptr, "AssetManager::Init called with null device");
        m_device = device;

        if (!m_fontLibrary.IsValid() && !m_fontLibrary.Init()) {
            KbkError(kLogChannel, "Failed to initialize FreeType font library");
        }
    }

    void AssetManager::Shutdown()
    {
        Clear();
        m_device = nullptr;
        m_fontLibrary.Shutdown();
        KbkLog(kLogChannel, "AssetManager shutdown");
    }

    void AssetManager::Clear()
    {
        m_textures.clear();
        m_fonts.clear();
    }

    Texture2D* AssetManager::LoadTexture(const std::string& id,
        const std::string& path,
        bool sRGB)
    {
        if (!m_device) {
            KbkError(kLogChannel,
                "Cannot load texture '%s' (id='%s'): device is null",
                path.c_str(), id.c_str());
            return nullptr;
        }

        auto it = m_textures.find(id);
        if (it != m_textures.end()) {
            KbkTrace(kLogChannel,
                "Reusing already loaded texture '%s' (id='%s')",
                path.c_str(), id.c_str());
            return it->second.get();
        }

        auto texture = std::make_unique<Texture2D>();
        if (!texture->LoadFromFile(m_device, path, sRGB)) {
            KbkError(kLogChannel,
                "Failed to load texture from '%s' (id='%s')",
                path.c_str(), id.c_str());
            return nullptr;
        }

        Texture2D* result = texture.get();
        m_textures.emplace(id, std::move(texture));

        KbkLog(kLogChannel,
            "Loaded texture '%s' as id='%s' (%dx%d)",
            path.c_str(), id.c_str(), result->Width(), result->Height());

        return result;
    }

    Font* AssetManager::LoadFontTTF(const std::string& id,
        const std::string& path,
        int pixelHeight)
    {
        if (!m_device) {
            KbkError(kLogChannel,
                "Cannot load font '%s' (id='%s'): device is null",
                path.c_str(), id.c_str());
            return nullptr;
        }

        if (!m_fontLibrary.IsValid()) {
            KbkError(kLogChannel,
                "Cannot load font '%s' (id='%s'): font library unavailable",
                path.c_str(), id.c_str());
            return nullptr;
        }

        auto it = m_fonts.find(id);
        if (it != m_fonts.end()) {
            KbkTrace(kLogChannel,
                "Reusing already loaded font '%s' (id='%s')",
                path.c_str(), id.c_str());
            return it->second.get();
        }

        auto font = m_fontLibrary.LoadFontFromFile(m_device, path, pixelHeight);
        if (!font) {
            KbkError(kLogChannel,
                "Failed to load font from '%s' (id='%s')",
                path.c_str(), id.c_str());
            return nullptr;
        }

        Font* result = font.get();
        m_fonts.emplace(id, std::move(font));
        return result;
    }

    Font* AssetManager::GetFont(const std::string& id)
    {
        auto it = m_fonts.find(id);
        if (it == m_fonts.end())
            return nullptr;
        return it->second.get();
    }

    const Font* AssetManager::GetFont(const std::string& id) const
    {
        auto it = m_fonts.find(id);
        if (it == m_fonts.end())
            return nullptr;
        return it->second.get();
    }

    Texture2D* AssetManager::GetTexture(const std::string& id)
    {
        auto it = m_textures.find(id);
        if (it == m_textures.end())
            return nullptr;
        return it->second.get();
    }

    const Texture2D* AssetManager::GetTexture(const std::string& id) const
    {
        auto it = m_textures.find(id);
        if (it == m_textures.end())
            return nullptr;
        return it->second.get();
    }

} // namespace KibakoEngine
