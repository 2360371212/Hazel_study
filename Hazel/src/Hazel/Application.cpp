#include "Application.h"
#include <iostream>

namespace Hazel {
    Application::Application() {

    }

    Application::~Application() {
    }

    void Application::Run() {
        while (true) {
            // Run the application
            std::cout << "Running application..." << std::endl;
        }
    }
}