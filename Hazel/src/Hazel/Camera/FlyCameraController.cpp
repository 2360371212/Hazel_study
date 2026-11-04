#include "FlyCameraController.h"
#include "PerspectiveCamera.h"
#include "Hazel/Input/Input.h"
#include "Hazel/Input/MouseButtonCodes.h"
#include "Hazel/Input/KeyCodes.h"
#include <glm/gtc/constants.hpp>

namespace Hazel{


    FlyCameraController::FlyCameraController(float verticalFOVRadians, float aspectRatio, float nearClip, float farClip)
        : m_Camera(verticalFOVRadians, aspectRatio, nearClip, farClip)
    {}

    void FlyCameraController::OnUpdate(Timestep deltaTime){
        if (!m_Enabled ||
            !Input::IsMouseButtonPressed(HZ_MOUSE_BUTTON_RIGHT))
        {
            m_Looking = false;
            return;
        }

        const auto [mouseX, mouseY] = Input::GetMousePosition();
        const glm::vec2 mousePosition{ mouseX, mouseY };

        if (!m_Looking)
        {
            // 刚按下右键时只记录起点，避免视角突然跳转。
            m_LastMousePosition = mousePosition;
            m_Looking = true;
        }

        // 确定鼠标移动方向
        const glm::vec2 mouseDelta = mousePosition - m_LastMousePosition;

        m_LastMousePosition = mousePosition;

        // 先旋转，再使用新的朝向移动。
        UpdateRotation(mouseDelta);
        UpdateTranslation(deltaTime);
    }

    void FlyCameraController::OnEvent(Event& e){
        EventDispatcher dispatcher(e);

        dispatcher.Dispatch<WindowResizeEvent>(
            HZ_BIND_EVENT_FN(FlyCameraController::OnWindowResized));

        dispatcher.Dispatch<MouseButtonReleasedEvent>(
            HZ_BIND_EVENT_FN(FlyCameraController::OnMouseButtonReleased));

        dispatcher.Dispatch<MouseScrolledEvent>(
            HZ_BIND_EVENT_FN(FlyCameraController::OnMouseScrolled));
    }

    void FlyCameraController::SetEnabled(bool enabled){
        if (m_Enabled != enabled)
            m_Looking = false;

        m_Enabled = enabled;
    }

    void FlyCameraController::SetMoveSpeed(float unitsPerSecond){
        m_MoveSpeed = unitsPerSecond;
    }
    
    void FlyCameraController::SetMouseSensitivity(float radiansPerPixel){
        m_MouseSensitivity = radiansPerPixel;
    }

    void FlyCameraController::UpdateRotation(const glm::vec2& mouseDelta){
        glm::vec3 rotation = m_Camera.GetRotation();
        rotation.y -= mouseDelta.x * m_MouseSensitivity;
        rotation.x -= mouseDelta.y * m_MouseSensitivity;

        const float pitchLimit = glm::radians(89.0f);

        rotation.x = std::clamp(rotation.x, -pitchLimit, pitchLimit);

        // 保持 yaw 在有限范围内，避免长时间累计。
        rotation.y = std::remainder(rotation.y, glm::two_pi<float>());
        
        rotation.z = 0.0f;
        m_Camera.SetRotation(rotation);
    }

    void FlyCameraController::UpdateTranslation(Timestep deltaTime){
        const float deltaSeconds = deltaTime;
        glm::vec3 direction{0.0f};

        const glm::vec3& forward = m_Camera.GetForwardDirection();
        const glm::vec3& right = m_Camera.GetRightDirection();

        const glm::vec3 worldUp{ 0.0f, 1.0f, 0.0f };

        if (Input::IsKeyPressed(HZ_KEY_W))
            direction += forward;

        if (Input::IsKeyPressed(HZ_KEY_S))
            direction -= forward;

        if (Input::IsKeyPressed(HZ_KEY_D))
            direction += right;

        if (Input::IsKeyPressed(HZ_KEY_A))
            direction -= right;

        if (Input::IsKeyPressed(HZ_KEY_E))
            direction += worldUp;

        if (Input::IsKeyPressed(HZ_KEY_Q))
            direction -= worldUp;

        const float lengthSquared = glm::dot(direction, direction);

        direction /= std::sqrt(lengthSquared);

        float speed = m_MoveSpeed;

        if (Input::IsKeyPressed(HZ_KEY_LEFT_SHIFT) ||
            Input::IsKeyPressed(HZ_KEY_RIGHT_SHIFT))
        {
            speed *= m_BoostMultiplier;
        }

        const glm::vec3 position = m_Camera.GetPosition() + direction * speed * deltaSeconds;

        m_Camera.SetPosition(position);
    }

    bool FlyCameraController::OnMouseScrolled(MouseScrolledEvent& event){
        const float scroll = std::clamp(
            event.GetYOffset(),
            -10.0f,
            10.0f
        );
        // 按住右键：调整飞行速度。
        if (Input::IsMouseButtonPressed(HZ_MOUSE_BUTTON_RIGHT))
        {
            m_MoveSpeed *= std::pow(1.2f, scroll);
            m_MoveSpeed = std::clamp(m_MoveSpeed, 0.1f, 100.0f);
            return true;
        }

        // 不按右键：沿相机观察方向推近、拉远。
        const glm::vec3 position = m_Camera.GetPosition() + m_Camera.GetForwardDirection() * scroll * m_ScrollMoveStep;
        m_Camera.SetPosition(position);
        return true;
    }
    bool FlyCameraController::OnMouseButtonReleased(MouseButtonReleasedEvent& event){
        if (event.GetMouseButton() == HZ_MOUSE_BUTTON_RIGHT)
            m_Looking = false;
        return false;
    }
    bool FlyCameraController::OnWindowResized(WindowResizeEvent& event){
        m_Camera.SetViewportSize(event.GetWidth(), event.GetHeight());
        return false;
    }
}