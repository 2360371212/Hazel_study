#include "hzpch.h"
#include "MaterialBinder.h"

#include <cassert>
#include <string>
#include <type_traits>
#include <variant>

#include "MaterialInstance.h"
#include "Hazel/Renderer/Common/Shader.h"
#include "Hazel/Core/Log.h"

namespace Hazel
{
    namespace
    {
        void UploadValue(Shader& shader, const std::string& name, const ShaderParameterValue& parameter)
        {
            std::visit([&](auto value)
                {
                    using T = std::decay_t<decltype(value)>;

                    if constexpr (std::is_same_v<T, int>)
                    {
                        shader.SetInt(name, value);
                    }
                    else if constexpr (std::is_same_v<T, float>)
                    {
                        shader.SetFloat(name, value);
                    }
                    else if constexpr (std::is_same_v<T, glm::vec2>)
                    {
                        shader.SetFloat2(name, value);
                    }
                    else if constexpr (std::is_same_v<T, glm::vec3>)
                    {
                        shader.SetFloat3(name, value);
                    }
                    else if constexpr (std::is_same_v<T, glm::vec4>)
                    {
                        shader.SetFloat4(name, value);
                    }
                    else if constexpr (std::is_same_v<T, glm::mat3>)
                    {
                        shader.SetMat3(name, value);
                    }
                    else if constexpr (std::is_same_v<T, glm::mat4>)
                    {
                        shader.SetMat4(name, value);
                    }
                }, parameter);
        }
    }


    uint32_t MaterialBinder::Bind(const MaterialInstance& instance, uint32_t firstTextureSlot, uint32_t textureSlotLimit)
    {


        const auto& material = *instance.GetMaterial();
        const auto& defaults = material.GetDefaultShaderParams();
        // 获取个性参数
        const auto& overrides = instance.GetOverrides();

        HZ_CORE_ASSERT(defaults.GetTextures().size() <= (textureSlotLimit - firstTextureSlot), "the count of textures is out of scope")

        auto& shader = *material.GetShader();

        shader.Bind();

        // 每次上传完整材质参数：
        // 有覆盖值就使用覆盖值，否则使用默认值。
        for (const auto& [name, defaultValue] : defaults.GetValues())
        {
            const auto* overrideValue = overrides.Find(name);
            UploadValue(shader, name, overrideValue ? *overrideValue : defaultValue);
        }



        uint32_t slot = firstTextureSlot;

        // 绑定纹理
        for (const auto& [name, defaultTexture] : defaults.GetTextures())
        {
            const auto* overrideTexture = overrides.FindTexture(name);
            const auto& texture = overrideTexture ? *overrideTexture : defaultTexture;

            texture->Bind(slot);
            shader.SetInt(name, static_cast<int>(slot));
            ++slot;
        }
        return slot;
    }
}