#pragma once

#include <optional>
#include <string>

#include "Material.h"

namespace Hazel
{
    // 在共享的基础上增加自己的材质模板
    class MaterialInstance
    {
    public:
        explicit MaterialInstance(Ref<Material> material);

        void Set(const std::string& name, ShaderParameterValue value);
        void SetTexture(const std::string& name, const Ref<Texture2D>& texture);
        void ResetParameter(const std::string& name);
        void ResetTexture(const std::string& name);
        void SetRenderState(const RenderState& state);

        void ResetRenderState();
        void ResetAllOverrides();

        const Ref<Material>& GetMaterial() const
        {
            return m_Material;
        }

        const ShaderParameters& GetOverrides() const
        {
            return m_Overrides;
        }

        const RenderState& GetRenderState() const;

    private:
        Ref<Material> m_Material;
        ShaderParameters m_Overrides;
        std::optional<RenderState> m_RenderStateOverride;
    };
}