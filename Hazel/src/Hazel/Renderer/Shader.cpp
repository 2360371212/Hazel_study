#include "hzpch.h"
#include "Shader.h"
#include "Hazel/Core/Log.h"
#include "glad/glad.h" 
#include "Hazel/Renderer/Renderer.h"
#include "Platform/OpenGL/OpenGLShader.h"

namespace Hazel{
    Ref<Shader> Shader::Create(const std::string& name, const std::string& vertexSrc, const std::string& fragmentSrc){
        switch (Renderer::GetAPI())
        {
            case RendererAPI::API::None:{
                HZ_CORE_ASSERT(false, "RendererAPI currently not supported");
            }
            case RendererAPI::API::OpenGL:{
                return std::make_shared<OpenGLShader>(name, vertexSrc, fragmentSrc);
            }
        }
        HZ_CORE_ASSERT(false, "UnKnow RendererAPI!");
        return nullptr;
    }


    Ref<Shader> Shader::Create(const std::string& filepath){
        switch (Renderer::GetAPI())
        {
            case RendererAPI::API::None:{
                HZ_CORE_ASSERT(false, "RendererAPI currently not supported");
            }
            case RendererAPI::API::OpenGL:{
                return std::make_shared<OpenGLShader>(filepath);
            }
        }
        HZ_CORE_ASSERT(false, "UnKnow RendererAPI!");
        return nullptr;
    }


    void ShaderLibrary::Add(const Ref<Shader>& shader){
        auto& name = shader->GetName(); 
        HZ_CORE_ASSERT(m_Shaders.find(name) == m_Shaders.end(), "Shader already exists!");
        m_Shaders[name] = shader;
    }

    void ShaderLibrary::Add(const std::string& name, const Ref<Shader>& shader){
        HZ_CORE_ASSERT(m_Shaders.find(name) == m_Shaders.end(), "Shader already exists!");
        m_Shaders[name] = shader;
    }

    Ref<Shader> ShaderLibrary::Load(const std::string& filepath){
        auto shader = Ref<Shader>(Shader::Create(filepath));
        Add(shader);
        return shader;
    }  

    Ref<Shader> ShaderLibrary::Load(const std::string& name, const std::string& filepath){
        auto shader = Ref<Shader>(Shader::Create(filepath));
        Add(name, shader);
        return shader;
    }

    Ref<Shader> ShaderLibrary::Get(const std::string& name){
        HZ_CORE_ASSERT(m_Shaders.find(name) != m_Shaders.end(), "Shader not found!");
        return m_Shaders[name];
    }
}
