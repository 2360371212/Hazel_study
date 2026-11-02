#include "Sandbox2D.h"
#include "imgui.h"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

#include "Platform/OpenGL/OpenGLShader.h"


Sandbox2D::Sandbox2D()
    : Layer("Sandbox2D"), m_CameraController(1280.0f / 720.0f, true){
}


void Sandbox2D::OnAttach(){
    m_CheckerboardTexture = Hazel::Texture2D::Create("D:/Project/cpp/Hazel/Sandbox/assets/textures/test.jpg");
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
    Hazel::Renderer2D::DrawQuad({-1.f, 0.f},{0.8f, 0.8f}, {0.2, 0.1, 0.8f, 1.f});
    Hazel::Renderer2D::DrawQuad({0.5f, -0.5f, 0.1f}, {0.75f, 0.75f}, m_CheckerboardTexture);
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