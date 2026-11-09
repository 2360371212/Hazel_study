#include "hzpch.h"
#include "Framebuffer.h"

#include "Hazel/Renderer/Renderer.h"
#include "Platform/OpenGL/OpenGLFramebuffer.h"

namespace Hazel
{
    Ref<Framebuffer> Framebuffer::Create(const FramebufferSpecification& specification)
    {
        switch (RendererAPI::GetAPI())
        {
            case RendererAPI::API::OpenGL:
                return CreateRef<OpenGLFramebuffer>(specification);
            case RendererAPI::API::None:
                break;
        }
        HZ_CORE_ASSERT(false, "Unsupported renderer API");
        return nullptr;
    }
}