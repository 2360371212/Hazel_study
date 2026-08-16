#include "Hazel.h"

class Sandbox : public Hazel::Application {
    public:
        Sandbox() {

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
