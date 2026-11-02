#include "Renderer2D.h"
#include "hzpch.h"
#include "Hazel/Renderer/VertexArray.h"
#include "Hazel/Renderer/Shader.h"
#include "Hazel/Renderer/OrthographicCamera.h"
#include <glm/gtc/matrix_transform.hpp>
#include "Hazel/Renderer/RenderCommand.h"

namespace Hazel{

    struct Renderer2DStorage{
        Ref<VertexArray> QuadVertexArray;
        Ref<Shader> TextureShader;
        Ref<Texture2D> WhiteTexture;
    };

    static Renderer2DStorage* s_Data;

    void Renderer2D::Init(){
        s_Data = new Renderer2DStorage();

        s_Data->QuadVertexArray = VertexArray::Create();

        float squareVertices[4 * 5] = {
            -0.5f, -0.5f, 0.0f, 0.0f, 0.0f,
            0.5f, -0.5f, 0.0f, 1.0f, 0.0f,
            0.5f, 0.5f, 0.0f, 1.0f, 1.0f, 
            -0.5f, 0.5f, 0.0f, 0.0f, 1.0f
        };
            
        Ref<VertexBuffer> squareVB;
        squareVB.reset(VertexBuffer::Create(squareVertices, sizeof(squareVertices)));

        BufferLayout squareLayout= {
            {ShaderDataType::Float3, "a_Position"},
            {ShaderDataType::Float2, "a_TexCoord"},
        };

        squareVB->SetLayout(squareLayout);
            
        uint32_t squareIndices[6] = {0, 1, 2, 2, 3, 0};
        Ref<IndexBuffer> squareIB;
        squareIB.reset(IndexBuffer::Create(squareIndices, sizeof(squareIndices)/ sizeof(uint32_t)));

        s_Data->QuadVertexArray->AddVertexBuffer(squareVB);
        s_Data->QuadVertexArray->SetIndexBuffer(squareIB);

        s_Data->WhiteTexture = Texture2D::Create(1, 1);
        uint32_t whiteTextureData = 0xffffffff;
        s_Data->WhiteTexture->SetData(&whiteTextureData, sizeof(uint32_t));


        // 纹理着色器
        s_Data->TextureShader = Shader::Create("D:/Project/cpp/Hazel/Sandbox/assets/shaders/Texture.glsl");
        s_Data->TextureShader->Bind();
        s_Data->TextureShader->SetInt("u_Texture", 0);

    }

    void Renderer2D::Shutdown(){
        delete s_Data;
    }

    void Renderer2D::BeginScene(const OrthographicCamera& camera){
        s_Data->TextureShader->Bind();
        s_Data->TextureShader->SetMat4("u_ViewProjection", camera.GetViewProjectionMatrix());
    }

    void Renderer2D::EndScene(){

    }

    void Renderer2D::DrawQuad(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color){
        DrawQuad(glm::vec3(position, 0.0f), size, color);
    }

    void Renderer2D::DrawQuad(const glm::vec3& position, const glm::vec2& size, const glm::vec4& color){

        s_Data->TextureShader->SetFloat4("u_Color", color);
        // Bind white Texture
        s_Data->WhiteTexture->Bind();


        // 变换矩阵 = 平移*旋转*缩放
        glm::mat4 transform = glm::translate(glm::mat4(1.0f), position) * glm::scale(glm::mat4(1.0f), glm::vec3(size, 1.0f));

        s_Data->TextureShader->SetMat4("u_Transform", transform);
        s_Data->QuadVertexArray->Bind();
        RenderCommand::DrawIndexed(s_Data->QuadVertexArray);
    }

    void Renderer2D::DrawQuad(const glm::vec2& position, const glm::vec2& size, const Ref<Texture>& texture){
        DrawQuad(glm::vec3(position, 0.0f), size, texture);
    }

    void Renderer2D::DrawQuad(const glm::vec3& position, const glm::vec2& size, const Ref<Texture>& texture){
        // 绑定白色
        s_Data->TextureShader->SetFloat4("u_Color", glm::vec4(1.0f));

        // 绑定纹理单元
        texture->Bind();

        // 变换矩阵 = 平移*旋转*缩放
        glm::mat4 transform = glm::translate(glm::mat4(1.0f), position) * glm::scale(glm::mat4(1.0f), glm::vec3(size, 1.0f));
        s_Data->TextureShader->SetMat4("u_Transform", transform);

        s_Data->QuadVertexArray->Bind();

        RenderCommand::DrawIndexed(s_Data->QuadVertexArray);
    }
}