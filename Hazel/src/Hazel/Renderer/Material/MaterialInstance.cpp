#include "hzpch.h"
#include "MaterialInstance.h"
#include "Hazel/Core/Log.h"
#include <cassert>
#include <utility>

namespace Hazel
{
    MaterialInstance::MaterialInstance(Ref<Material> material) 
        : m_Material(std::move(material)) {}

    void MaterialInstance::Set(const std::string& name, ShaderParameterValue value)
    {
        // const auto* defaultValue = m_Material->GetShaderParams().Find(name);
        m_Overrides.Set(name, std::move(value));
    }

    void MaterialInstance::SetTexture(const std::string& name, const Ref<Texture2D>& texture)
    {
        m_Overrides.SetTexture(name, texture);
    }

    void MaterialInstance::ResetParameter(const std::string& name)
    {
        m_Overrides.Remove(name);
    }

    void MaterialInstance::ResetTexture(const std::string& name)
    {
        m_Overrides.RemoveTexture(name);
    }

    void MaterialInstance::SetRenderState(const RenderState& state)
    {
        m_RenderStateOverride = state;
    }

    void MaterialInstance::ResetRenderState()
    {
        m_RenderStateOverride.reset();
    }

    void MaterialInstance::ResetAllOverrides()
    {
        m_Overrides.Clear();
        m_RenderStateOverride.reset();
    }

    const RenderState& MaterialInstance::GetRenderState() const
    {
        if (m_RenderStateOverride)
            return *m_RenderStateOverride;

        return m_Material->GetRenderState();
    }
}