#include "hzpch.h"
#include "Shader.h"
#include "Hazel/Log.h"
#include "glad/glad.h" 
#include "Hazel/Renderer/Renderer.h"
#include "Platform/OpenGL/OpenGLShader.h"

namespace Hazel{
    Shader* Shader::Create(const std::string& vertexSrc, const std::string& fragmentSrc){
        switch (Renderer::GetAPI())
        {
            case RendererAPI::API::None:{
                HZ_CORE_ASSERT(false, "RendererAPI currently not supported");
            }
            case RendererAPI::API::OpenGL:{
                return new OpenGLShader(vertexSrc, fragmentSrc);
            }
        }
        HZ_CORE_ASSERT(false, "UnKnow RendererAPI!");
        return nullptr;
    }
}
