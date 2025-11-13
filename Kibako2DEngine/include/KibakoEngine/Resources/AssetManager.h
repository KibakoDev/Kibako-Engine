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

        // Doit être appelé une fois après la création du RendererD3D11
        void Init(ID3D11Device* device);

        // Libère toutes les ressources chargées
        void Shutdown();

        // Charge une texture si elle n'existe pas déjà pour cet id
        // Retourne toujours le pointeur stocké (nouveau ou existant)
        Texture2D* LoadTexture(const std::string& id,
            const std::string& path,
            bool sRGB = true);

        // Retourne nullptr si aucune texture avec cet id
        Texture2D* GetTexture(const std::string& id);
        const Texture2D* GetTexture(const std::string& id) const;

        // Supprime toutes les textures chargées (appelé par Shutdown)
        void Clear();

    private:
        ID3D11Device* m_device = nullptr;

        std::unordered_map<std::string, std::unique_ptr<Texture2D>> m_textures;
    };

} // namespace KibakoEngine