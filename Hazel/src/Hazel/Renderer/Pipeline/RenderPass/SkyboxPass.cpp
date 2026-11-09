#include "hzpch.h"
#include "SkyboxPass.h"

#include <glm/glm.hpp>

#include "Hazel/Renderer/Common/RenderCommand.h"
#include "Hazel/Renderer/Common/RenderState.h"

namespace Hazel
{
    SkyboxPass::SkyboxPass(const Ref<Shader>& shader, const Ref<FullscreenTriangle>& fullscreenTriangle)
        : m_Shader(shader), m_FullscreenTriangle(fullscreenTriangle)
    {}

    void SkyboxPass::Execute(const RenderPassContext& context)
    {
        const auto& environment = context.Scene.Environment;
        const auto& view = context.View;

        // 管线已经确认背景启用，并且 SkyboxMap 有效。

        RenderState state;
        state.DepthTest = false;
        state.DepthWrite = false;
        state.Cull = CullMode::None;
        state.Blend = BlendMode::Opaque;

        RenderCommand::SetRenderState(state);

        // 从屏幕位置重建观察方向。
        const glm::mat4 inverseProjection = glm::inverse(view.Projection);

        // 相机视图矩阵仅取旋转部分。
        // 对不含缩放的相机，旋转矩阵的逆等于转置。
        const glm::mat3 inverseViewRotation = glm::transpose(glm::mat3(view.View));

        m_Shader->Bind();

        m_Shader->SetMat4("u_InverseProjection", inverseProjection);
        m_Shader->SetMat3("u_InverseViewRotation", inverseViewRotation);
        m_Shader->SetFloat("u_EnvironmentRotation", environment.RotationRadians);
        m_Shader->SetFloat("u_Intensity", environment.BackgroundIntensity);
        environment.SkyboxMap->Bind(0);
        m_Shader->SetInt("u_EnvironmentMap", 0);
        
        m_FullscreenTriangle->Draw();
    }
}
