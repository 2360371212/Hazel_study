#include "hzpch.h"
#include "Texture.h"
#include "Hazel/Renderer/Renderer.h"
#include "Platform/OpenGL/OpenGLTexture.h"
#include <memory>

namespace Hazel{
    Ref<Texture2D> Texture2D::Create(const std::string& path){
        switch (Renderer::GetAPI())
        {
            case RendererAPI::API::None:{
                HZ_CORE_ASSERT(false, "RendererAPI currently not supported");
            }
            case RendererAPI::API::OpenGL:{
                return std::make_shared<OpenGLTexture2D>(path);
            }
        }
        HZ_CORE_ASSERT(false, "UnKnow RendererAPI!");
        return nullptr;
    }

    Ref<Texture2D> Texture2D::Create(uint32_t width, uint32_t height){
        switch (Renderer::GetAPI())
        {
            case RendererAPI::API::None:{
                HZ_CORE_ASSERT(false, "RendererAPI currently not supported");
            }
            case RendererAPI::API::OpenGL:{
                return std::make_shared<OpenGLTexture2D>(width, height);
            }
        }
        HZ_CORE_ASSERT(false, "UnKnow RendererAPI!");
        return nullptr;
    }
}