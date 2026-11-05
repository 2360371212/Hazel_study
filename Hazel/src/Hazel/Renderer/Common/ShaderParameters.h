#pragma once
#include <variant>
#include <glm/glm.hpp>
#include <map>

#include "Hazel/Renderer/Common/Texture.h"

// 用于创建一批固定的材质参数(用于Shader)

namespace Hazel{

    using ShaderParameterValue = std::variant<
        int,
        float,
        glm::vec1,
        glm::vec2,
        glm::vec3,
        glm::vec4,
        glm::mat2,
        glm::mat3
    >;

    class ShaderParameters{
        public:
            using ValueMap = std::map<std::string, ShaderParameterValue>;
            using TextureMap = std::map<std::string, Ref<Texture2D>>;

        public:
            void Set(const std::string& name, ShaderParameterValue value);
            void SetTexture(const std::string& name, const Ref<Texture2D>& texture);

            const ShaderParameterValue* Find(const std::string& name) const;
            const Ref<Texture2D>* FindTexture(const std::string& name) const;

            void Remove(const std::string& name);
            void RemoveTexture(const std::string& name);

            void Clear();

            const ValueMap& GetValues() const
            {
                return m_Values;
            }

            const TextureMap& GetTextures() const
            {
                return m_Textures;
            }

        private:
            ValueMap m_Values;
            TextureMap m_Textures;
    };
}