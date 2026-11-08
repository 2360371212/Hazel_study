#include "hzpch.h"
#include "Texture.h"
#include "Hazel/Renderer/Common/RendererAPI.h"
#include "Platform/OpenGL/OpenGLTexture.h"
#include <memory>
#include <stdexcept>

namespace Hazel{
    Ref<Texture2D> Texture2D::Create(const std::string& path)
    {
        return Create(path, TextureLoadOptions{});
    }

    Ref<Texture2D> Texture2D::Create(const std::string& path, const TextureLoadOptions& options)
    {
        switch (RendererAPI::GetAPI())
        {
            case RendererAPI::API::OpenGL:
                return std::make_shared<OpenGLTexture2D>(path, options);
        }

        throw std::runtime_error("Renderer API does not support textures");
    }

    Ref<Texture2D> Texture2D::Create(uint32_t width, uint32_t height)
    {
        TextureSpecification specification;
        specification.Width = width;
        specification.Height = height;
        specification.Wrap = TextureWrap::Repeat;

        return Create(specification);
    }

    Ref<Texture2D> Texture2D::Create(const TextureSpecification& specification)
    {
        switch (RendererAPI::GetAPI())
        {
            case RendererAPI::API::OpenGL:{
                return std::make_shared<OpenGLTexture2D>(specification);
            }
        }

        throw std::runtime_error("Renderer API does not support textures");
    }

    Ref<TextureCube> TextureCube::Create(const TextureSpecification& specification)
    {
        switch (RendererAPI::GetAPI())
        {
            case RendererAPI::API::OpenGL:{
                return std::make_shared<OpenGLTextureCube>(specification);
            }
        }

        throw std::runtime_error("Renderer API does not support textures");
    }
}
