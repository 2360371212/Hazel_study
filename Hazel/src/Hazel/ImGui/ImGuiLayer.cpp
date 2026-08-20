#include "hzpch.h"
#include "ImGuiLayer.h"
#include "Hazel/Core.h"
#include "Platform/OpenGL/ImGuiOpenGLRenderer.h"
#include "Hazel/Application.h"


#include "GLFW/glfw3.h"
#include <glad/glad.h>

namespace Hazel{
    static ImGuiKey GLFWKeyToImGuiKey(int keyCode);
    // topest layer
    ImGuiLayer::ImGuiLayer()
        :Layer("ImGuiLayer") {
    }

    ImGuiLayer::~ImGuiLayer(){

    }

    void ImGuiLayer::OnUpdate() {

        ImGuiIO& io = ImGui::GetIO();
        Application& app = Application::Get();
        io.DisplaySize = ImVec2(app.GetWindow().GetWidth(), app.GetWindow().GetHeight());

        float time = (float)glfwGetTime();
        io.DeltaTime = m_Time> 0.0f ? (time-m_Time) : (1.0f / 60.0f);
        m_Time = time;

        ImGui_ImplOpenGL3_NewFrame();
        ImGui::NewFrame();
        
        static bool show = true;
        ImGui::ShowDemoWindow(&show);

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }

    void ImGuiLayer::OnAttach() {
        ImGui::CreateContext();
        ImGui::StyleColorsDark();

        ImGuiIO& io = ImGui::GetIO();

        ImGuiStyle& style = ImGui::GetStyle();

        constexpr float uiScale = 1.5f;

        // 放大字体
        style.FontScaleMain = uiScale;

        // 放大按钮、间距、标题栏等控件
        style.ScaleAllSizes(uiScale);

        // 使用新版可缩放的内置字体，放大后比旧位图字体更清晰
        io.Fonts->AddFontDefaultVector();

        io.BackendFlags |= ImGuiBackendFlags_HasMouseCursors;
        io.BackendFlags |= ImGuiBackendFlags_HasSetMousePos;


        ImGui_ImplOpenGL3_Init("#version 410");
    }

    void ImGuiLayer::OnDetach(){

    }
    
    void ImGuiLayer::OnEvent(Event& event){
        EventDispatcher dispatcher(event);
        dispatcher.Dispatch<MouseButtonPressedEvent>(HZ_BIND_EVENT_FN(ImGuiLayer::OnMouseButtonPressedEvent));
        dispatcher.Dispatch<MouseButtonReleasedEvent>(HZ_BIND_EVENT_FN(ImGuiLayer::OnMouseButtonReleasedEvent));
        dispatcher.Dispatch<MouseMovedEvent>(HZ_BIND_EVENT_FN(ImGuiLayer::OnMouseMovedEvent));
        dispatcher.Dispatch<MouseScrolledEvent>(HZ_BIND_EVENT_FN(ImGuiLayer::OnMouseScrolledEvent));
        dispatcher.Dispatch<KeyPressedEvent>(HZ_BIND_EVENT_FN(ImGuiLayer::OnKeyPressedEvent));
        dispatcher.Dispatch<KeyReleasedEvent>(HZ_BIND_EVENT_FN(ImGuiLayer::OnKeyReleasedEvent));
        dispatcher.Dispatch<KeyTypedEvent>(HZ_BIND_EVENT_FN(ImGuiLayer::OnKeyTypedEvent));
        dispatcher.Dispatch<WindowResizeEvent>(HZ_BIND_EVENT_FN(ImGuiLayer::OnWindowResizeEvent)); 
    }

    bool ImGuiLayer::OnMouseButtonPressedEvent(MouseButtonPressedEvent& event) {
        ImGuiIO& io = ImGui::GetIO();
        io.MouseDown[event.GetMouseButton()] = true;
        return false;
    }

    bool ImGuiLayer::OnMouseButtonReleasedEvent(MouseButtonReleasedEvent& event) {
        ImGuiIO& io = ImGui::GetIO();
        io.MouseDown[event.GetMouseButton()] = false;
        return false;
    }

    bool ImGuiLayer::OnMouseMovedEvent(MouseMovedEvent& event) {
        ImGuiIO& io = ImGui::GetIO();
        io.MousePos = ImVec2(event.GetX(), event.GetY());
        return false;
    }

    bool ImGuiLayer::OnMouseScrolledEvent(MouseScrolledEvent& event) {
        ImGuiIO& io = ImGui::GetIO();
        io.MouseWheelH += event.GetXOffset();
        io.MouseWheel += event.GetYOffset();
        return false;
    }

    bool ImGuiLayer::OnKeyPressedEvent(KeyPressedEvent& event) {
        ImGuiIO& io = ImGui::GetIO();

        const ImGuiKey key = GLFWKeyToImGuiKey(event.GetKeyCode());

        if (key != ImGuiKey_None){
            if(event.GetRepeatCount() == 0)
                io.AddKeyEvent(key, true);
        }
        return false;
    }

    bool ImGuiLayer::OnKeyReleasedEvent(KeyReleasedEvent& event) {
        ImGuiIO& io = ImGui::GetIO();

        const ImGuiKey key = GLFWKeyToImGuiKey(event.GetKeyCode());
        if (key != ImGuiKey_None){
            io.AddKeyEvent(key, false);
        }
        return false;
    }

    bool ImGuiLayer::OnKeyTypedEvent(KeyTypedEvent& event) {
        ImGuiIO& io = ImGui::GetIO();
        int KeyCode = event.GetKeyCode();
        if (KeyCode > 0 && KeyCode < 0x10000)
            io.AddInputCharacter((unsigned short)KeyCode);
        return false;
    }

    bool ImGuiLayer::OnWindowResizeEvent(WindowResizeEvent& event) {
        ImGuiIO& io = ImGui::GetIO();
        io.DisplaySize = ImVec2((float)event.GetWidth(), (float)event.GetHeight());
        io.DisplayFramebufferScale = ImVec2(1.0f, 1.0f);

        glViewport(0,0, event.GetWidth(), event.GetHeight());

        return false;
    }

    static ImGuiKey GLFWKeyToImGuiKey(int keyCode)
    {
        switch (keyCode)
        {
            case GLFW_KEY_TAB:           return ImGuiKey_Tab;
            case GLFW_KEY_LEFT:          return ImGuiKey_LeftArrow;
            case GLFW_KEY_RIGHT:         return ImGuiKey_RightArrow;
            case GLFW_KEY_UP:            return ImGuiKey_UpArrow;
            case GLFW_KEY_DOWN:          return ImGuiKey_DownArrow;
            case GLFW_KEY_PAGE_UP:       return ImGuiKey_PageUp;
            case GLFW_KEY_PAGE_DOWN:     return ImGuiKey_PageDown;
            case GLFW_KEY_HOME:          return ImGuiKey_Home;
            case GLFW_KEY_END:           return ImGuiKey_End;
            case GLFW_KEY_INSERT:        return ImGuiKey_Insert;
            case GLFW_KEY_DELETE:        return ImGuiKey_Delete;
            case GLFW_KEY_BACKSPACE:     return ImGuiKey_Backspace;
            case GLFW_KEY_SPACE:         return ImGuiKey_Space;
            case GLFW_KEY_ENTER:         return ImGuiKey_Enter;
            case GLFW_KEY_ESCAPE:        return ImGuiKey_Escape;

            case GLFW_KEY_A:             return ImGuiKey_A;
            case GLFW_KEY_C:             return ImGuiKey_C;
            case GLFW_KEY_V:             return ImGuiKey_V;
            case GLFW_KEY_X:             return ImGuiKey_X;
            case GLFW_KEY_Y:             return ImGuiKey_Y;
            case GLFW_KEY_Z:             return ImGuiKey_Z;

            case GLFW_KEY_LEFT_CONTROL:  return ImGuiKey_LeftCtrl;
            case GLFW_KEY_RIGHT_CONTROL: return ImGuiKey_RightCtrl;
            case GLFW_KEY_LEFT_SHIFT:    return ImGuiKey_LeftShift;
            case GLFW_KEY_RIGHT_SHIFT:   return ImGuiKey_RightShift;
            case GLFW_KEY_LEFT_ALT:      return ImGuiKey_LeftAlt;
            case GLFW_KEY_RIGHT_ALT:     return ImGuiKey_RightAlt;
            case GLFW_KEY_LEFT_SUPER:    return ImGuiKey_LeftSuper;
            case GLFW_KEY_RIGHT_SUPER:   return ImGuiKey_RightSuper;

            default:
                return ImGuiKey_None;
        }
    }

}
