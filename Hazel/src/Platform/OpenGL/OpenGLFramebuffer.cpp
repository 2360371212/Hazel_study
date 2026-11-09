#include "hzpch.h"
#include "OpenGLFramebuffer.h"
#include "Hazel/Core/Log.h"
#include <stdexcept>

namespace Hazel
{
    namespace
    {
        bool IsDepthFormat(TextureFormat format)
        {
            return format == TextureFormat::Depth24Stencil8 || format == TextureFormat::Depth32F;
        }
    }

    OpenGLFramebuffer::OpenGLFramebuffer(const FramebufferSpecification& specification)
        : m_Specification(specification)
    {
        Invalidate();
    }

    OpenGLFramebuffer::~OpenGLFramebuffer()
    {
        DestroyAttachments();

        if (m_RendererID != 0)
        {
            glDeleteFramebuffers(1, &m_RendererID);
            m_RendererID = 0;
        }
    }

    void OpenGLFramebuffer::Bind()
    {
        glBindFramebuffer(GL_FRAMEBUFFER, m_RendererID);
        glViewport(0, 0, static_cast<GLsizei>(m_Specification.Width), static_cast<GLsizei>(m_Specification.Height));
    }

    void OpenGLFramebuffer::Unbind()
    {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    void OpenGLFramebuffer::Resize(uint32_t width, uint32_t height)
    {
        if (width == 0 || height == 0)
            return;
        m_Specification.Width = width;
        m_Specification.Height = height;
        Invalidate();
    }

    void OpenGLFramebuffer::Invalidate()
    {
        DestroyAttachments();

        if (m_RendererID == 0)
        {
            glCreateFramebuffers(1, &m_RendererID);
        }

        CreateAttachments();
        glBindFramebuffer(GL_FRAMEBUFFER, m_RendererID);
        std::vector<GLenum> drawBuffers;

        for (uint32_t i = 0; i < m_ColorAttachments.size(); ++i)
        {
            const GLenum attachment = GetColorAttachmentPoint(i);
            auto& colorAttachment = m_ColorAttachments[i];

            glNamedFramebufferTexture(m_RendererID, attachment, colorAttachment->GetRendererID(), 0);

            drawBuffers.push_back(attachment);
        }

        if (m_DepthAttachment)
        {
            const TextureFormat format = m_DepthAttachment->GetFormat();
            const GLenum attachment = format == TextureFormat::Depth24Stencil8 ? GL_DEPTH_STENCIL_ATTACHMENT : GL_DEPTH_ATTACHMENT;
            glNamedFramebufferTexture(m_RendererID, attachment, m_DepthAttachment->GetRendererID(), 0);
        }

        if (drawBuffers.empty())
        {
            glNamedFramebufferDrawBuffer(m_RendererID, GL_NONE);
            glNamedFramebufferReadBuffer(m_RendererID, GL_NONE);
        }
        else
        {
            glNamedFramebufferDrawBuffers(m_RendererID, static_cast<GLsizei>(drawBuffers.size()), drawBuffers.data());
        }

        if (!IsComplete())
        {
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            HZ_CORE_ASSERT(false ,"OpenGL framebuffer is incomplete")
        }

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    void OpenGLFramebuffer::CreateAttachments()
    {
        for (const auto& attachment : m_Specification.Attachments)
        {
            if (IsDepthFormat(attachment.Format))
            {
                if (m_DepthAttachment)
                {
                    HZ_CORE_ASSERT(false, "Only one depth attachment is supported")
                }

                TextureSpecification textureSpec;
                textureSpec.Width = m_Specification.Width;
                textureSpec.Height = m_Specification.Height;
                textureSpec.Format = attachment.Format;
                textureSpec.MipLevels = 1;
                textureSpec.Wrap = TextureWrap::ClampToEdge;
                m_DepthAttachment = Texture2D::Create(textureSpec);
            }
            else
            {
                TextureSpecification textureSpec;

                textureSpec.Width = m_Specification.Width;

                textureSpec.Height = m_Specification.Height;

                textureSpec.Format = attachment.Format;

                textureSpec.MipLevels = 1;
                textureSpec.Wrap = TextureWrap::ClampToEdge;

                m_ColorAttachments.push_back(Texture2D::Create(textureSpec));
            }
        }

        if (m_Specification.HasDepthAttachment && !m_DepthAttachment)
        {
            TextureSpecification depthSpec;

            depthSpec.Width = m_Specification.Width;

            depthSpec.Height = m_Specification.Height;

            depthSpec.Format = m_Specification.HasStencilAttachment ? TextureFormat::Depth24Stencil8 : TextureFormat::Depth32F;

            depthSpec.MipLevels = 1;
            depthSpec.Wrap = TextureWrap::ClampToEdge;

            m_DepthAttachment = Texture2D::Create(depthSpec);
        }
    }

    void OpenGLFramebuffer::DestroyAttachments()
    {
        m_ColorAttachments.clear();
        m_DepthAttachment.reset();
    }

    GLenum OpenGLFramebuffer::GetColorAttachmentPoint(uint32_t index) const
    {
        return GL_COLOR_ATTACHMENT0 + index;
    }

    Ref<Texture2D> OpenGLFramebuffer::GetColorAttachment(uint32_t index) const
    {
        if (index >= m_ColorAttachments.size())
            return nullptr;
        return m_ColorAttachments[index];
    }

    Ref<Texture2D> OpenGLFramebuffer::GetDepthAttachment() const
    {
        return m_DepthAttachment;
    }

    bool OpenGLFramebuffer::IsComplete() const
    {
        return glCheckNamedFramebufferStatus(m_RendererID, GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE;
    }

    void OpenGLFramebuffer::ClearColorAttachment(uint32_t index, const glm::vec4& color)
    {
        if (index >= m_ColorAttachments.size())
            return;

        const float values[4] = {
            color.r,
            color.g,
            color.b,
            color.a
        };

        glClearNamedFramebufferfv(m_RendererID, GL_COLOR, static_cast<GLint>(index), values);
    }
}