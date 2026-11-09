#include "hzpch.h"
#include "ForwardRenderPipeline.h"
#include <utility>
#include "Hazel/Renderer/Common/RenderCommand.h"

namespace Hazel
{
    ForwardRenderPipeline::ForwardRenderPipeline(Scope<RenderPass> skyboxPass, Scope<RenderPass> toneMappingPass)
        : m_SkyboxPass(std::move(skyboxPass)), m_ToneMappingPass(std::move(toneMappingPass))
    {}

    void ForwardRenderPipeline::AddScenePass(Scope<RenderPass> pass)
    {
        m_ScenePasses.emplace_back(std::move(pass));
    }

    // 确保管线已经拥有尺寸正确的 HDR Framebuffer
    void ForwardRenderPipeline::EnsureResources(uint32_t width, uint32_t height)
    {
        if (!m_Resources.SceneHDR)
        {
            FramebufferSpecification specification;
            specification.Width = width;
            specification.Height = height;

            specification.Attachments = {
                { TextureFormat::RGBA16F }
            };

            // 未显式指定深度格式时，当前 Framebuffer
            // 实现会根据这两个标志创建 Depth32F。
            specification.HasDepthAttachment = true;
            specification.HasStencilAttachment = false;

            m_Resources.SceneHDR = Framebuffer::Create(specification);

            return;
        }

        const auto& specification = m_Resources.SceneHDR->GetSpecification();

        if (specification.Width != width || specification.Height != height)
        {
            m_Resources.SceneHDR->Resize(width, height);
        }
    }

    void ForwardRenderPipeline::Render(const RenderSceneData& scene, const RenderView& view, const Ref<Framebuffer>& output){
        if (view.ViewportWidth == 0 || view.ViewportHeight == 0)
        {
            return;
        }

        EnsureResources(view.ViewportWidth, view.ViewportHeight);

        const RenderPassContext context{scene, view, m_Resources};

        // 1. 准备 HDR 场景目标。
        m_Resources.SceneHDR->Bind();

        RenderCommand::SetSRGBWrite(false);
        RenderCommand::SetScissorTest(false);
        RenderCommand::SetColorWrite(true, true, true, true);

        // 深度清除需要允许深度写入。
        RenderCommand::SetDepthWrite(true);
        RenderCommand::SetClearDepth(1.0f);
        RenderCommand::SetClearColor(scene.Environment.ClearColor);
        RenderCommand::Clear();

        // 2. 绘制天空背景。
        if (scene.Environment.ShowBackground && scene.Environment.SkyboxMap)
        {
            m_SkyboxPass->Execute(context);
        }

        // 3. 按顺序执行场景 Pass。
        for (const auto& pass : m_ScenePasses)
        {
            pass->Execute(context);
        }

        // 4. 切换最终输出目标。
        m_Resources.SceneHDR->Unbind();

        if (output)
        {
            output->Bind();
        }
        else
        {
            RenderCommand::SetViewport(0, 0, view.ViewportWidth, view.ViewportHeight);
        }

        // 最终编码由 ToneMapping Shader 完成。
        RenderCommand::SetSRGBWrite(false);

        // 5. 色调映射。
        m_ToneMappingPass->Execute(context);

        // 6. 返回默认输出目标及视口。
        if (output)
        {
            output->Unbind();
        }

        RenderCommand::SetViewport(0, 0, view.ViewportWidth, view.ViewportHeight);
    }

    Ref<Texture2D> ForwardRenderPipeline::GetHDRColor() const
    {
        if (!m_Resources.SceneHDR)
        {
            return nullptr;
        }
        
        return m_Resources.SceneHDR->GetColorAttachment(0);
    }
}
