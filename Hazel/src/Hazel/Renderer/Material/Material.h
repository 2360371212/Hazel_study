#pragma once

#include <string>

#include "Hazel/Core/Core.h"
#include "Hazel/Renderer/Common/RenderState.h"
#include "Hazel/Renderer/Common/Shader.h"
#include "Hazel/Renderer/Common/ShaderParameters.h"

namespace Hazel
{
    // 保存共享的材质模板
    class Material
    {
    public:
        Material(std::string name, Ref<Shader> shader, ShaderParameters defaultShaderParams, RenderState renderState = {});

        const std::string& GetName() const
        {
            return m_Name;
        }

        const Ref<Shader>& GetShader() const
        {
            return m_Shader;
        }

        const ShaderParameters& GetDefaultShaderParams() const
        {
            return m_DefaultShaderParams;
        }

        const RenderState& GetRenderState() const
        {
            return m_RenderState;
        }

    private:
        std::string m_Name;
        Ref<Shader> m_Shader;
        ShaderParameters m_DefaultShaderParams;
        RenderState m_RenderState;
    };
}