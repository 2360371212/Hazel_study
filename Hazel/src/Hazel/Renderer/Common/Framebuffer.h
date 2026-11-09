#pragma once

#include <cstdint>
#include <vector>
#include <glm/glm.hpp>
#include "Hazel/Core/Core.h"
#include "Hazel/Renderer/Common/Texture.h"

namespace Hazel
{
    struct FramebufferAttachmentSpecification
    {
        TextureFormat Format = TextureFormat::RGBA8;
    };

    struct FramebufferSpecification
    {
        uint32_t Width = 1280;
        uint32_t Height = 720;

        std::vector<FramebufferAttachmentSpecification> Attachments;

        bool HasDepthAttachment = true;
        bool HasStencilAttachment = false;
    };

    class Framebuffer
    {
        public:
            virtual ~Framebuffer() = default;

            virtual void Bind() = 0;
            virtual void Unbind() = 0;

            virtual void Resize(uint32_t width, uint32_t height) = 0;

            virtual void Invalidate() = 0;

            virtual const FramebufferSpecification& GetSpecification() const = 0;

            virtual Ref<Texture2D>
            GetColorAttachment(uint32_t index = 0) const = 0;

            virtual Ref<Texture2D>
            GetDepthAttachment() const = 0;

            virtual uint32_t GetColorAttachmentCount() const = 0;

            virtual bool IsComplete() const = 0;

            virtual void ClearColorAttachment(uint32_t index, const glm::vec4& color) = 0;

            static Ref<Framebuffer> Create(const FramebufferSpecification& specification);
    };
}