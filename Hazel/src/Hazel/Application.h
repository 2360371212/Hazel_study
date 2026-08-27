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

            // std::shared_ptr<Shader> m_Shader;
            // std::shared_ptr<VertexArray> m_VertexArray;
            // std::shared_ptr<VertexBuffer> m_VertexBuffer;
            // std::shared_ptr<IndexBuffer> m_IndexBuffer;

            // std::shared_ptr<Shader> m_SquareShader;
            // std::shared_ptr<VertexArray> m_SquareVA;
        protected:
            bool m_Running = true;
    };

    // To be defined in CLIENT
    Application* CreateApplication();
}