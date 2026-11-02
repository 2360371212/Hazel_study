#include "Renderer2D.h"
#include "hzpch.h"
#include "Hazel/Renderer/VertexArray.h"
#include "Hazel/Renderer/Shader.h"
#include "Hazel/Renderer/OrthographicCamera.h"

#include "Hazel/Renderer/RenderCommand.h"
#include "Platform/OpenGL/OpenGLShader.h"
namespace Hazel{

    struct Renderer2DStorage{
        Ref<VertexArray> m_SquareVA;
        Ref<Shader> m_FlatColorShader;
    };

    static Renderer2DStorage* s_Data;

    void Renderer2D::Init(){
        s_Data = new Renderer2DStorage();

        s_Data->m_SquareVA = VertexArray::Create();

        float squareVertices[4 * 5] = {
            -0.5f, -0.5f, 0.0f,
            0.5f, -0.5f, 0.0f,
            0.5f, 0.5f, 0.0f, 
            -0.5f, 0.5f, 0.0f, 
        };
            
        Ref<VertexBuffer> squareVB;
        squareVB.reset(VertexBuffer::Create(squareVertices, sizeof(squareVertices)));

        BufferLayout squareLayout= {
            {ShaderDataType::Float3, "a_Position"},
        };

        squareVB->SetLayout(squareLayout);
            
        uint32_t squareIndices[6] = {0, 1, 2, 2, 3, 0};
        Ref<IndexBuffer> squareIB;
        squareIB.reset(IndexBuffer::Create(squareIndices, sizeof(squareIndices)/ sizeof(uint32_t)));

        s_Data->m_SquareVA->AddVertexBuffer(squareVB);
        s_Data->m_SquareVA->SetIndexBuffer(squareIB);

        // 纯色着色器
        s_Data->m_FlatColorShader = Hazel::Shader::Create("D:/Project/cpp/Hazel/Sandbox/assets/shaders/FlatColorShader.glsl");
    }

    void Renderer2D::Shutdown(){
        delete s_Data;
    }

    void Renderer2D::BeginScene(const OrthographicCamera& camera){
        std::dynamic_pointer_cast<OpenGLShader>(s_Data->m_FlatColorShader)->Bind();
        std::dynamic_pointer_cast<OpenGLShader>(s_Data->m_FlatColorShader)->UploadUniformMat4("u_ViewProjection", camera.GetViewProjectionMatrix());
        std::dynamic_pointer_cast<OpenGLShader>(s_Data->m_FlatColorShader)->UploadUniformMat4("u_Transform", glm::mat4(1.0f));
    }

    void Renderer2D::EndScene(){

    }

    void Renderer2D::DrawQuad(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color){
        DrawQuad(glm::vec3(position, 0.0f), size, color);
    }

    void Renderer2D::DrawQuad(const glm::vec3& position, const glm::vec2& size, const glm::vec4& color){
        std::dynamic_pointer_cast<OpenGLShader>(s_Data->m_FlatColorShader)->Bind();
        std::dynamic_pointer_cast<OpenGLShader>(s_Data->m_FlatColorShader)->UploadUniformFloat4("u_Color", color);

        s_Data->m_SquareVA->Bind();
        RenderCommand::DrawIndexed(s_Data->m_SquareVA);
    }
}