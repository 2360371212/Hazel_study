#include "hzpch.h"
#include "Hazel/Core/Core.h"
#include "Buffer.h"
#include "Hazel/Renderer/Renderer.h"
#include "Hazel/Core/Log.h"
#include "Platform/OpenGL/OpenGLBuffer.h"



namespace Hazel{
    Ref<VertexBuffer> VertexBuffer::Create(float* vertices, uint32_t size) {
        switch (Renderer::GetAPI())
        {
            case RendererAPI::API::None :{
                HZ_CORE_ASSERT(false, "RendererAPI currently not supported");
            }
            case RendererAPI::API::OpenGL:{
                return CreateRef<OpenGLVertexBuffer>(vertices,size);
            }
        }
        HZ_CORE_ASSERT(false, "UnKnow RendererAPI!");
        return nullptr;
    }

    Ref<VertexBuffer> VertexBuffer::Create(uint32_t size)
	{
		switch (Renderer::GetAPI())
		{
			case RendererAPI::API::None:    HZ_CORE_ASSERT(false, "RendererAPI::None is currently not supported!"); return nullptr;
			case RendererAPI::API::OpenGL:  return CreateRef<OpenGLVertexBuffer>(size);
		}

		HZ_CORE_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}

    Ref<IndexBuffer> IndexBuffer::Create(uint32_t* indices, uint32_t count){
        switch (Renderer::GetAPI())
        {
            case RendererAPI::API::None:{
                HZ_CORE_ASSERT(false, "RendererAPI currently not supported");
            }
            case RendererAPI::API::OpenGL:{
                return CreateRef<OpenGLIndexBuffer>(indices, count);
            }
        }
        HZ_CORE_ASSERT(false, "UnKnow RendererAPI!");
        return nullptr;
    }
}