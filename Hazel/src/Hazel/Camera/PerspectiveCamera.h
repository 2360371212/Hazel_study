#pragma once

#include <glm/glm.hpp>

namespace Hazel{
    class PerspectiveCamera{
        public:
            PerspectiveCamera(float verticalFOVRadians, float aspectRatio, float nearClip, float farClip);

            void SetProjection(float verticalFOVRadians, float aspectRatio, float nearClip, float farClip);
            void SetViewportSize(uint32_t width, uint32_t height);
            void SetPosition(const glm::vec3& position);
            void SetRotation(const glm::vec3& rotationRadians);

            const glm::vec3& GetRotation() const
            {
                return m_RotationRadians;
            }

            const glm::vec3& GetPosition() const
            {
                return m_Position;
            }

            const glm::mat4& GetViewMatrix();
            const glm::mat4& GetProjectionMatrix();
            const glm::mat4& GetViewProjectionMatrix(); 

            const glm::vec3& GetForwardDirection() const;
            const glm::vec3& GetRightDirection() const;
            const glm::vec3& GetUpDirection() const;

        private:
            void RecalculateViewMatrix();
            void RecalculateProjectionMatrix();

            float m_VerticalFOVRadians;
            float m_AspectRatio;
            float m_NearClip;
            float m_FarClip;

            glm::vec3 m_Position{ 0.0f, 0.0f, 5.0f };
            glm::vec3 m_RotationRadians{ 0.0f }; // pitch、yaw、roll

            glm::vec3 m_Forward{0.0f, 0.0f, -1.0f};
            glm::vec3 m_Right{1.0f, 0.0f, 0.0f};
            glm::vec3 m_Up{0.0f, 1.0f, 0.0f};

            glm::mat4 m_ViewMatrix{1.0f};
            glm::mat4 m_ProjectionMatrix{1.0f};
            glm::mat4 m_ViewProjectionMatrix{1.0f}; 
    };
}