#include "hzpch.h"
#include "ShaderParameters.h"
#include "Hazel/Core/Log.h"

#include <cassert>
#include <utility>

namespace Hazel
{
    void ShaderParameters::Set(const std::string& name, ShaderParameterValue value)
    {
        // 没有就插入有就更新
        m_Values.insert_or_assign(name, std::move(value));
    }

    void ShaderParameters::SetTexture(const std::string& name, const Ref<Texture2D>& texture)
    {
        m_Textures.insert_or_assign(name, texture);
    }

    const ShaderParameterValue* ShaderParameters::Find(const std::string& name) const
    {
        const auto it = m_Values.find(name);
        return it != m_Values.end() ? &it->second : nullptr;
    }

    const Ref<Texture2D>* ShaderParameters::FindTexture(const std::string& name) const
    {
        const auto it = m_Textures.find(name);
        return it != m_Textures.end() ? &it->second : nullptr;
    }

    void ShaderParameters::Remove(const std::string& name)
    {
        m_Values.erase(name);
    }

    void ShaderParameters::RemoveTexture(const std::string& name)
    {
        m_Textures.erase(name);
    }

    void ShaderParameters::Clear()
    {
        m_Values.clear();
        m_Textures.clear();
    }
}