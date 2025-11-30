// Texture and font cache
#pragma once

#include <memory>
#include <string>
#include <unordered_map>

#include "KibakoEngine/Fonts/Font.h"
#include "KibakoEngine/Renderer/Texture2D.h"

struct ID3D11Device;

namespace KibakoEngine {

class AssetManager
{
public:
    AssetManager() = default;

    void Init(ID3D11Device* device);
    void Shutdown();

    // Load or fetch a texture
    [[nodiscard]] Texture2D* LoadTexture(const std::string& id,
                                         const std::string& path,
                                         bool sRGB = true);

    // Load or fetch a TTF font
    [[nodiscard]] Font* LoadFontTTF(const std::string& id,
                                    const std::string& path,
                                    int pixelHeight);

    [[nodiscard]] Font* GetFont(const std::string& id);
    [[nodiscard]] const Font* GetFont(const std::string& id) const;

    // Returns nullptr if missing
    [[nodiscard]] Texture2D* GetTexture(const std::string& id);
    [[nodiscard]] const Texture2D* GetTexture(const std::string& id) const;

    // Clear cached assets
    void Clear();

private:
    ID3D11Device* m_device = nullptr;

    FontLibrary m_fontLibrary;
    std::unordered_map<std::string, std::unique_ptr<Texture2D>> m_textures;
    std::unordered_map<std::string, std::unique_ptr<Font>> m_fonts;
};

} // namespace KibakoEngine

