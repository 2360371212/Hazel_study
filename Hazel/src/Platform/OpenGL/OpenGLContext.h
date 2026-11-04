#pragma once
#include "Hazel/Renderer/Common/GraphicsContext.h"
#include "GLFW/glfw3.h"

namespace Hazel{
    class OpenGLContext : public GraphicsContext{
        public:
            OpenGLContext(GLFWwindow* windowsHandle);
            virtual void Init() override;
            virtual void SwapBuffer() override;
        private:
            GLFWwindow* m_WindowsHandle;

    };
}