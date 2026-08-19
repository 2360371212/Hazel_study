#include "Hazel.h"

class ExampleLayer : public Hazel::Layer{
    public:
        ExampleLayer()
            : Layer("Example"){}
        
        void OnUpdate() override{
            HZ_INFO("ExapmleLayer::Update");
        }

        void OnEvent(Hazel::Event& event) override {
            HZ_TRACE("{0}", event.ToString());
        }

};



class Sandbox : public Hazel::Application {
    public:
        Sandbox() {
            PushLayer(new ExampleLayer());
        }

        ~Sandbox() {

        }

        void Run() {
            while (true);
        }
};

Hazel::Application* Hazel::CreateApplication() {
    return new Sandbox();
}
