#include "hzpch.h"
#include "Application.h"
#include "Hazel/Log.h"
#include "Input.h"
#include "Hazel/Renderer/Renderer.h"
#include "glm/glm.hpp"
#include "GLFW/glfw3.h"

namespace Hazel
{

    #define BIND_EVENT_FN(x) std::bind(&x , this, std::placeholders::_1)

    Application* Application::s_Instance = nullptr;


    Application::Application()
    {
        HZ_CORE_ASSERT(!s_Instance, "Application is already exists!");
        s_Instance = this;

        m_Window = std::unique_ptr<Window>(Window::Create());
        m_Window->SetEventCallback(BIND_EVENT_FN(Application::OnEvent));
        
        Renderer::Init();

        m_ImGuiLayer = new ImGuiLayer();
        PushOverlay(m_ImGuiLayer);

    }

    Application::~Application()
    {
    }

    void Application::OnEvent(Event& e){
        EventDispatcher dispatcher(e);
        dispatcher.Dispatch<WindowCloseEvent>(BIND_EVENT_FN(Application::OnWindowClosed));
        dispatcher.Dispatch<WindowResizeEvent>(BIND_EVENT_FN(Application::OnWindowResized));

        for(auto it = m_LayerStack.end(); it != m_LayerStack.begin();){
            (*--it)->OnEvent(e);
            if (e.m_Handled)
                break;
        }
    }

    void Application::Run()
    {
        while (m_Running)
        {
            float time = (float)glfwGetTime(); // Platform Time
            Timestep timestep = time - m_LastFrameTime;
            m_LastFrameTime = time;            

            // 每一层进行更新
            if (!m_Minimized) {
                for(Layer* layer : m_LayerStack){
                    layer->OnUpdate(timestep);
                }
            }
            m_ImGuiLayer->Begin();
            // ImGUI层更新
            for (Layer* layer : m_LayerStack)
                layer->OnImGuiRender();
            m_ImGuiLayer->End();
            m_Window->OnUpdate();
        }
    }

    void Application::PushLayer(Layer* layer){
        m_LayerStack.PushLayer(layer);
        layer->OnAttach();
    }

    void Application::PushOverlay(Layer* layer){
        m_LayerStack.PushOverlay(layer);
        layer->OnAttach();
    }

    bool Application::OnWindowClosed(WindowCloseEvent& e){
        m_Running = false;
        return true;
    }


    bool Application::OnWindowResized(WindowResizeEvent& e){
        if (e.GetWidth() == 0 || e.GetHeight() == 0){
            m_Minimized = true;
            return false;
        }
        m_Minimized = false;
        
        Renderer::OnWindowResize(e.GetWidth(), e.GetHeight());
        return false;
    }
}
