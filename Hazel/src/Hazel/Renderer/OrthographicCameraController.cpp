#include "hzpch.h"
#include "Hazel/Core/Input.h"
#include "Hazel/Core/Application.h"
#include "Hazel/Core/KeyCodes.h"
#include "OrthographicCameraController.h"

namespace Hazel{
    OrthographicCameraController::OrthographicCameraController(float aspectRatio, bool rotation)
        : m_AspectRatio(aspectRatio), m_Rotation(rotation), m_Camera(-m_AspectRatio * m_ZoomLevel, m_AspectRatio * m_ZoomLevel, -m_ZoomLevel, m_ZoomLevel)
    {
    } 



    void OrthographicCameraController::OnUpdate(Timestep deltaTime)
    {
        if (Input::IsKeyPressed(HZ_KEY_A)){
                m_CameraPosition.x -= m_CameraMoveSpeed * deltaTime;
        } else if (Hazel::Input::IsKeyPressed(HZ_KEY_D)){
                m_CameraPosition.x += m_CameraMoveSpeed * deltaTime;
        }
        if (Input::IsKeyPressed(HZ_KEY_W)){
                m_CameraPosition.y += m_CameraMoveSpeed * deltaTime;
        } else if (Hazel::Input::IsKeyPressed(HZ_KEY_S)){
                m_CameraPosition.y -= m_CameraMoveSpeed * deltaTime;
        }
        // 设置摄像机位置
        m_Camera.SetPosition(m_CameraPosition);
        if(m_Rotation){
            if (Input::IsKeyPressed(HZ_KEY_Q)){
                m_CameraRotation -= m_CameraRotationSpeed * deltaTime;
            }
            if (Input::IsKeyPressed(HZ_KEY_E)){
                m_CameraRotation += m_CameraRotationSpeed * deltaTime;
            }
            m_Camera.SetRotation(m_CameraRotation);
        }
    }

    void OrthographicCameraController::OnEvent(Event& e)
    {
        EventDispatcher dispatcher(e);
        dispatcher.Dispatch<MouseScrolledEvent>(HZ_BIND_EVENT_FN(OrthographicCameraController::OnMouseScrolled));
        dispatcher.Dispatch<WindowResizeEvent>(HZ_BIND_EVENT_FN(OrthographicCameraController::OnWindowResized));
    }

    bool OrthographicCameraController::OnMouseScrolled(MouseScrolledEvent& e)
    {
        // 鼠标滚轮改变缩放级别
        m_ZoomLevel  -= e.GetYOffset() * 0.5f;
        m_ZoomLevel = std::max(m_ZoomLevel, 0.25f);
        m_Camera.SetProjection(-m_AspectRatio * m_ZoomLevel, m_AspectRatio * m_ZoomLevel, -m_ZoomLevel, m_ZoomLevel);
        return false;
    }

    bool OrthographicCameraController::OnWindowResized(WindowResizeEvent& e)
    {
        m_AspectRatio = (float)e.GetWidth() / (float)e.GetHeight();
        m_Camera.SetProjection(-m_AspectRatio * m_ZoomLevel, m_AspectRatio * m_ZoomLevel, -m_ZoomLevel, m_ZoomLevel);
        return false;
    }
}