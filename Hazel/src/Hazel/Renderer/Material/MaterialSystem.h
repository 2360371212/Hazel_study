#pragma once

#include "Material.h"

namespace Hazel
{
    class MaterialSystem
    {
    public:
        MaterialSystem();

        Ref<Material> CreateUnlit(const Ref<Shader>& shader) const;
        Ref<Material> EmissiveMaterial(const Ref<Shader>& shader) const;
        Ref<Material> CreateBlinnPhong(const Ref<Shader>& shader) const;
        Ref<Material> CreatePBR(const Ref<Shader>& shader) const;

        const Ref<Texture2D>& GetWhiteTexture() const
        {
            return m_WhiteTexture;
        }

    private:
        Ref<Texture2D> m_WhiteTexture;
    };
}