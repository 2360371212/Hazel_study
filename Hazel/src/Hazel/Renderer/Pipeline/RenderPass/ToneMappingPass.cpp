#include "hzpch.h"
#include "ToneMappingPass.h"

#include "Hazel/Renderer/Common/RenderCommand.h"
#include "Hazel/Renderer/Common/RenderState.h"

namespace Hazel
{
    ToneMappingPass::ToneMappingPass(const Ref<Shader>& shader, const Ref<FullscreenTriangle>& fullscreenTriangle)
        : m_Shader(shader), m_FullscreenTriangle(fullscreenTriangle)
    {}

    void ToneMappingPass::Execute(const RenderPassContext& context)
    {
        RenderState state;
        state.DepthTest = false;
        state.DepthWrite = false;
        state.Cull = CullMode::None;
        state.Blend = BlendMode::Opaque;

        RenderCommand::SetRenderState(state);

        const auto hdrColor = context.Resources.SceneHDR->GetColorAttachment(0);

        m_Shader->Bind();
        hdrColor->Bind(0);
        m_Shader->SetInt("u_HDRColor", 0);
        m_Shader->SetFloat("u_Exposure", context.Scene.Environment.Exposure);
        m_FullscreenTriangle->Draw();
    }
}