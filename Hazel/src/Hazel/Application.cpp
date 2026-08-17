#include "Application.h"
#include "Hazel/Log.h"
#include <iostream>
#include "Hazel/Events/ApplicationEvent.h"

namespace Hazel {
    Application::Application() {

    }

    Application::~Application() {
    }

    void Application::Run() {
        WindowResizeEvent e(1280, 720); 
        HZ_TRACE("{}", e.ToString());

        while (true) {
            // Run the application
            // std::cout << "Running application..." << std::endl;
        }
    }
}