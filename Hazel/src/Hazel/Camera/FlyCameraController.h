#pragma once

#include "Hazel/Camera/PerspectiveCamera.h"
#include "Hazel/Core/Timestep.h"

#include "Hazel/Events/ApplicationEvent.h"
#include "Hazel/Events/MouseEvent.h"

namespace Hazel{
    class FlyCameraController{
        public:
            FlyCameraController(float verticalFOVRadians, float aspectRatio, float nearClip, float farClip);
            void OnUpdate(Timestep deltaTime);
            void OnEvent(Event& e);

            void SetEnabled(bool enabled);

            bool IsEnabled() const
            {
                return m_Enabled;
            }

            void SetMoveSpeed(float unitsPerSecond);
            void SetMouseSensitivity(float radiansPerPixel);

        private:
            void UpdateRotation(const glm::vec2& mouseDelta);
            void UpdateTranslation(Timestep deltaTime);

            bool OnMouseScrolled(MouseScrolledEvent& event);
            bool OnMouseButtonReleased(MouseButtonReleasedEvent& event);
            bool OnWindowResized(WindowResizeEvent& event);

        
        private:
            PerspectiveCamera m_Camera;
            
            // 每单位滚轮偏移，对应的相机移动距离。
            float m_ScrollMoveStep = 0.5f;

            // 世界单位 / 秒
            float m_MoveSpeed = 5.0f;
            float m_BoostMultiplier = 3.0f;

            // 鼠标视角灵敏度
            float m_MouseSensitivity = 0.003f;

            bool m_Enabled = true;
            // 鼠标查看视角
            bool m_Looking = false;

            glm::vec2 m_LastMousePosition{ 0.0f };
    };
}