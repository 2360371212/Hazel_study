#pragma once
#include "Core.h"
#include "Window.h"
#include "Hazel/LayerStack.h"

#include "Hazel/Events/Event.h"
#include "Hazel/Events/ApplicationEvent.h"
#include "Hazel/ImGui/ImGuiLayer.h"
#include "Hazel/Renderer/Shader.h"
#include "Hazel/Renderer/Buffer.h"
#include "Hazel/Renderer/VertexArray.h"

namespace Hazel {
    class HAZEL_API Application{
        public:
            Application();
            virtual ~Application();
            virtual void Run();

            void OnEvent(Event& e);

            void PushLayer(Layer* layer);
            void PushOverlay(Layer* overlay);

            inline Window& GetWindow(){return *m_Window;};

            inline static Application& Get() {return *s_Instance;}
            
        private:
            bool OnWindowClosed(WindowCloseEvent& e);

            std::unique_ptr<Window> m_Window;
            ImGuiLayer* m_ImGuiLayer;

            LayerStack m_LayerStack;

            static Application* s_Instance;


        protected:
            bool m_Running = true;
    };

    // To be defined in CLIENT
    Application* CreateApplication();
}