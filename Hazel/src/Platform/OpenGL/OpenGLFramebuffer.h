#pragma once

#include <vector>

#include <glad/glad.h>

#include "Hazel/Renderer/Common/Framebuffer.h"

namespace Hazel
{
    class OpenGLFramebuffer final : public Framebuffer
    {
        public:
            explicit OpenGLFramebuffer(const FramebufferSpecification& specification);

            ~OpenGLFramebuffer() override;

            void Bind() override;
            void Unbind() override;

            void Resize(uint32_t width, uint32_t height) override;
            void Invalidate() override;

            const FramebufferSpecification& GetSpecification() const override
            {
                return m_Specification;
            }

            Ref<Texture2D> GetColorAttachment(uint32_t index = 0) const override;

            Ref<Texture2D> GetDepthAttachment() const override;

            uint32_t GetColorAttachmentCount() const override
            {
                return static_cast<uint32_t>(m_ColorAttachments.size());
            }

            bool IsComplete() const override;

            void ClearColorAttachment(uint32_t index, const glm::vec4& color) override;

        private:
            void CreateAttachments();
            void DestroyAttachments();

            GLenum GetColorAttachmentPoint(uint32_t index) const;

        private:
            uint32_t m_RendererID = 0;
            FramebufferSpecification m_Specification;
            std::vector<Ref<Texture2D>> m_ColorAttachments;
            Ref<Texture2D> m_DepthAttachment;
    };
}