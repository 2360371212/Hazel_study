#pragma once
#include "Hazel/Camera/OrthographicCamera.h"
#include "Hazel/Core/Timestep.h"

#include "Hazel/Events/ApplicationEvent.h"
#include "Hazel/Events/MouseEvent.h"


namespace Hazel{
    class OrthographicCameraController{
        public:
            OrthographicCameraController(float aspectRatio, bool rotation = false);  // aspectRatio: 宽高比，rotation: 是否旋转
            
            void OnUpdate(Timestep deltaTime);
            void OnEvent(Event& e);

            OrthographicCamera& GetCamera() {return m_Camera;}
            const OrthographicCamera& GetCamera() const {return m_Camera;}

        private:
            bool OnMouseScrolled(MouseScrolledEvent& e);
            bool OnWindowResized(WindowResizeEvent& e);

        private:
            float m_AspectRatio;
            bool m_Rotation;
            float m_ZoomLevel = 1.0f;
            OrthographicCamera m_Camera;

            glm::vec3 m_CameraPosition = {0.0f, 0.0f, 0.0f};
            float m_CameraRotation = 0.0f;
            float m_CameraMoveSpeed = 5.0f, m_CameraRotationSpeed = 180.0f;
    };
}