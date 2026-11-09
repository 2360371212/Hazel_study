#pragma once

#include "Hazel/Renderer/Pipeline/RenderPass.h"
#include "Hazel/Renderer/Common/Shader.h"
#include "Hazel/Renderer/Common/VertexArray.h"
#include "Hazel/Renderer/Common/FullscreenTriangle.h"

namespace Hazel
{
    class HAZEL_API ToneMappingPass final : public RenderPass
    {
        public:
            // shader 和 fullscreenTriangle 必须有效。
            ToneMappingPass(const Ref<Shader>& shader, const Ref<FullscreenTriangle>& fullscreenTriangle);

            const char* GetName() const override
            {
                return "ToneMappingPass";
            }

            void Execute(const RenderPassContext& context) override;

        private:
            Ref<Shader> m_Shader;
            Ref<FullscreenTriangle> m_FullscreenTriangle;
    };
}