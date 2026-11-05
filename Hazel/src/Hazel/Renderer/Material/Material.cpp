#include "hzpch.h"
#include "Material.h"

#include <cassert>
#include <utility>

namespace Hazel
{
    Material::Material(std::string name, Ref<Shader> shader, ShaderParameters defaultShaderParams, RenderState renderState)
        : m_Name(std::move(name)), m_Shader(std::move(shader)), m_DefaultShaderParams(std::move(defaultShaderParams)), m_RenderState(renderState)
    {}
}