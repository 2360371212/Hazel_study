#include "Sandbox2D.h"
#include "imgui.h"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

#include "Platform/OpenGL/OpenGLShader.h"


Sandbox2D::Sandbox2D()
    : Layer("Sandbox2D"), m_CameraController(1280.0f / 720.0f, true){
}


void Sandbox2D::OnAttach(){
        m_SquareVA = Hazel::VertexArray::Create();

        float squareVertices[4 * 5] = {
            -0.5f, -0.5f, 0.0f,
            0.5f, -0.5f, 0.0f,
            0.5f, 0.5f, 0.0f, 
            -0.5f, 0.5f, 0.0f, 
        };
            
        Hazel::Ref<Hazel::VertexBuffer> squareVB;
        squareVB.reset(Hazel::VertexBuffer::Create(squareVertices, sizeof(squareVertices)));

        Hazel::BufferLayout squareLayout= {
            {Hazel::ShaderDataType::Float3, "a_Position"},
        };

        squareVB->SetLayout(squareLayout);
            
        uint32_t squareIndices[6] = {0, 1, 2, 2, 3, 0};
        Hazel::Ref<Hazel::IndexBuffer> squareIB;
        squareIB.reset(Hazel::IndexBuffer::Create(squareIndices, sizeof(squareIndices)/ sizeof(uint32_t)));

        m_SquareVA->AddVertexBuffer(squareVB);
        m_SquareVA->SetIndexBuffer(squareIB);


            // 纯色着色器
        m_FlatColorShader = Hazel::Shader::Create("D:/Project/cpp/Hazel/Sandbox/assets/shaders/FlatColorShader.glsl");
}

void Sandbox2D::OnDetach(){

}

void Sandbox2D::OnUpdate(Hazel::Timestep deltaTime){
    // Update
    m_CameraController.OnUpdate(deltaTime);

    // Render
    Hazel::RenderCommand::SetClearColor({0.1f, 0.1f, 0.1f, 1});
    Hazel::RenderCommand::Clear();

    Hazel::Renderer2D::BeginScene(m_CameraController.GetCamera());
    Hazel::Renderer2D::DrawQuad(glm::vec3(0.0f), glm::vec2(1.0f), m_SquareColor);
    Hazel::Renderer2D::EndScene();

    // TODO
    // std::dynamic_pointer_cast<Hazel::OpenGLShader>(m_FlatColorShader)->Bind();
    // std::dynamic_pointer_cast<Hazel::OpenGLShader>(m_FlatColorShader)->UploadUniformFloat4("u_Color", m_SquareColor);
}

void Sandbox2D::OnImGuiRender(){
    ImGui::Begin("Settings");
    ImGui::ColorEdit4("Square Color", glm::value_ptr(m_SquareColor));
    ImGui::End();
}

void Sandbox2D::OnEvent(Hazel::Event& event){
    m_CameraController.OnEvent(event);
}