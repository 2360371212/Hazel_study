#include "hzpch.h"
#include "Hazel/Log.h"
#include "OpenGLContext.h"
#include <glad/glad.h>

namespace Hazel{
    OpenGLContext::OpenGLContext(GLFWwindow* windowsHandle)
        : m_WindowsHandle(windowsHandle){
            HZ_CORE_ASSERT(windowsHandle, "Window handle is null")
        }

    void OpenGLContext::Init(){
        glfwMakeContextCurrent(m_WindowsHandle);
        int status = gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
        HZ_CORE_ASSERT(status, "Failed to initialize Glad!");
    }

    void OpenGLContext::SwapBuffer(){
        glfwSwapBuffers(m_WindowsHandle);
    }
}
