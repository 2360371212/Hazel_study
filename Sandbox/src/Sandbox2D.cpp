#include "Sandbox2D.h"
#include "imgui.h"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

#include "Platform/OpenGL/OpenGLShader.h"
#include <chrono>

template<typename Fn>
class Timer{
    public:
        // Timer(const char* name)
        //     :m_Name(name), m_Stopped(false)
        // {
        //     m_StartTime = std::chrono::high_resolution_clock::now();
        // }

        Timer(const char* name, Fn&& callback)
            :m_Name(name), m_Stopped(false), m_Callback(callback)
        {
            m_StartTime = std::chrono::high_resolution_clock::now();
        }

        ~Timer(){
            if(!m_Stopped){
                Stop();
            }
        }

        void Stop(){
            auto endTime = std::chrono::high_resolution_clock::now();
            auto start = std::chrono::time_point_cast<std::chrono::microseconds>(m_StartTime).time_since_epoch().count();
            auto end = std::chrono::time_point_cast<std::chrono::microseconds>(endTime).time_since_epoch().count();

            float duration = (end - start) * 0.001f;

            m_Stopped = true;
            m_Callback({m_Name, duration});
        }

    private:
        const char* m_Name;
        std::chrono::time_point<std::chrono::steady_clock> m_StartTime;
        bool m_Stopped;
        Fn m_Callback;
};

#define PROFILE_SCOPE(name) Timer timer##__LINE__(name, [&](ProfileResult profileResult){m_ProfileResults.push_back(profileResult);});

Sandbox2D::Sandbox2D()
    : Layer("Sandbox2D"), m_CameraController(1280.0f / 720.0f, true){
}


void Sandbox2D::OnAttach(){
    m_CheckerboardTexture = Hazel::Texture2D::Create("D:/Project/cpp/Hazel/Sandbox/assets/textures/test.jpg");
}

void Sandbox2D::OnDetach(){

}

void Sandbox2D::OnUpdate(Hazel::Timestep deltaTime){

    PROFILE_SCOPE("Sandbox2D::OnUpdate");

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

    for (auto& result : m_ProfileResults){
        ImGui::Text("%s: %.3fms", result.Name, result.Time);
    }

    m_ProfileResults.clear();

    ImGui::End();
}

void Sandbox2D::OnEvent(Hazel::Event& event){
    m_CameraController.OnEvent(event);
}