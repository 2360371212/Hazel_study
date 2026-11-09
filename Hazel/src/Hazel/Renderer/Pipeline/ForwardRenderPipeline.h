#pragma once

#include <vector>

#include "Hazel/Renderer/Pipeline/RenderPipeline.h"
#include "Hazel/Renderer/Pipeline/RenderPass.h"
#include "Hazel/Renderer/Pipeline/RenderFrameResources.h"

namespace Hazel
{
    class HAZEL_API ForwardRenderPipeline final : public RenderPipeline
    {
    public:
        // 构造后，它们的生命周期由管线管理。
        ForwardRenderPipeline(Scope<RenderPass> skyboxPass, Scope<RenderPass> toneMappingPass);

        // 按添加顺序，在天空盒之后、色调映射之前执行。
        // pass 必须有效。
        void AddScenePass(Scope<RenderPass> pass);
        void Render(const RenderSceneData& scene, const RenderView& view, const Ref<Framebuffer>& output = nullptr) override;

        Ref<Texture2D> GetHDRColor() const override;
    private:
        void EnsureResources(uint32_t width, uint32_t height);

    private:
        RenderFrameResources m_Resources;
        Scope<RenderPass> m_SkyboxPass;
        std::vector<Scope<RenderPass>> m_ScenePasses;
        Scope<RenderPass> m_ToneMappingPass;
    };
}