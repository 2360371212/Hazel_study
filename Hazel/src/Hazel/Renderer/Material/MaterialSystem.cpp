#include "hzpch.h"
#include "MaterialSystem.h"

#include <cstdint>
#include <utility>
#include <glm/glm.hpp>

namespace Hazel
{
    MaterialSystem::MaterialSystem()
    {
        m_WhiteTexture = Texture2D::Create(1, 1);
        uint8_t whitePixel[4] = {255, 255, 255, 255};
        m_WhiteTexture->SetData(whitePixel, sizeof(whitePixel));
    }
 
    // 基础材质模型
    Ref<Material> MaterialSystem::CreateUnlit(const Ref<Shader>& shader) const
    {
        ShaderParameters defaults;
        defaults.Set("u_Material.BaseColor", glm::vec4(1.0f));
        defaults.SetTexture("u_BaseColorMap", m_WhiteTexture);

        return CreateRef<Material>("Unlit", shader, std::move(defaults));
    }

    // blinnPhong 模型
    Ref<Material> MaterialSystem::CreateBlinnPhong(const Ref<Shader>& shader) const
    {
        ShaderParameters defaults;

        defaults.Set("u_Material.BaseColor", glm::vec4(1.0f));
        defaults.Set("u_Material.SpecularStrength", 0.3f);
        defaults.Set("u_Material.Shininess", 32.0f);
        defaults.SetTexture("u_BaseColorMap", m_WhiteTexture);

        return CreateRef<Material>("BlinnPhong", shader, std::move(defaults));
    }


    Ref<Material> MaterialSystem::CreatePBR(const Ref<Shader>& shader) const
    {
        ShaderParameters defaults;

        defaults.Set("u_Material.BaseColor", glm::vec4(1.0f));
        defaults.Set("u_Material.Metallic", 0.0f);
        defaults.Set("u_Material.Roughness", 0.5f);
        defaults.SetTexture("u_BaseColorMap", m_WhiteTexture);

        return CreateRef<Material>("PBR", shader, std::move(defaults));
    }

}