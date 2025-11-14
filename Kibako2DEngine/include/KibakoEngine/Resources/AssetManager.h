#pragma once

#include <memory>
#include <string>
#include <unordered_map>

#include "KibakoEngine/Renderer/Texture2D.h"

struct ID3D11Device;

namespace KibakoEngine {

class AssetManager
{
public:
    AssetManager() = default;

    void Init(ID3D11Device* device);
    void Shutdown();

    // Loads a texture if it does not already exist for the provided id
    // Always returns the stored pointer (newly created or cached)
    [[nodiscard]] Texture2D* LoadTexture(const std::string& id,
                                         const std::string& path,
                                         bool sRGB = true);

    // Returns nullptr when no texture has been loaded with this id
    [[nodiscard]] Texture2D* GetTexture(const std::string& id);
    [[nodiscard]] const Texture2D* GetTexture(const std::string& id) const;

    // Removes all cached textures (invoked by Shutdown)
    void Clear();

private:
    ID3D11Device* m_device = nullptr;

    std::unordered_map<std::string, std::unique_ptr<Texture2D>> m_textures;
};

} // namespace KibakoEngine

