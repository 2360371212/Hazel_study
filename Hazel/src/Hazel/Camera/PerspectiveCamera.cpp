#include "hzpch.h"
#include "PerspectiveCamera.h"

#include <glm/ext/matrix_clip_space.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

namespace Hazel{
    PerspectiveCamera::PerspectiveCamera(float verticalFOVRadians, float aspectRatio, float nearClip, float farClip){
        SetProjection(verticalFOVRadians, aspectRatio, nearClip, farClip);
        RecalculateViewMatrix();
    }

    void PerspectiveCamera::SetProjection(float verticalFOVRadians, float aspectRatio, float nearClip, float farClip){
        m_VerticalFOVRadians = verticalFOVRadians;
        m_AspectRatio = aspectRatio;
        m_NearClip = nearClip;
        m_FarClip = farClip;
        RecalculateProjectionMatrix();
    }

    void PerspectiveCamera::SetViewportSize(uint32_t width, uint32_t height){
        const float aspectRatio = static_cast<float>(width) / static_cast<float>(height);
        SetProjection(m_VerticalFOVRadians, aspectRatio, m_NearClip, m_FarClip);
    }
    
    void PerspectiveCamera::SetPosition(const glm::vec3& position){
        m_Position = position;
        RecalculateViewMatrix();
    }

    void PerspectiveCamera::SetRotation(const glm::vec3& rotationRadians){
        m_RotationRadians = rotationRadians;
        RecalculateViewMatrix();
    }

    const glm::mat4& PerspectiveCamera::GetViewMatrix(){
        return m_ViewMatrix;
    }
    const glm::mat4& PerspectiveCamera::GetProjectionMatrix(){
        return m_ProjectionMatrix;
    }
    const glm::mat4& PerspectiveCamera::GetViewProjectionMatrix(){
        return m_ViewProjectionMatrix;
    }

    const glm::vec3& PerspectiveCamera::GetForwardDirection() const{
        return m_Forward; 
    }

    const glm::vec3& PerspectiveCamera::GetRightDirection() const{
        return m_Right;
    } 
            
    const glm::vec3& PerspectiveCamera::GetUpDirection() const{
        return m_Up;
    }

    void PerspectiveCamera::RecalculateViewMatrix(){
        const float pitch = m_RotationRadians.x;
        const float yaw = m_RotationRadians.y;
        const float roll = m_RotationRadians.z;      
        
        const glm::quat yawRotation = glm::angleAxis(yaw, glm::vec3(0.0f, 1.0f, 0.0f));
        const glm::quat pitchRotation = glm::angleAxis(pitch, glm::vec3(1.0f, 0.0f, 0.0f));
        const glm::quat rollRotation = glm::angleAxis(roll, glm::vec3(0.0f, 0.0f, 1.0f));
        const glm::quat orientation = glm::normalize(yawRotation * pitchRotation * rollRotation);

        m_Forward = orientation * glm::vec3(0.0f, 0.0f, -1.0f);
        m_Right = orientation * glm::vec3(1.0f, 0.0f, 0.0f);
        m_Up = orientation * glm::vec3(0.0f, 1.0f, 0.0f);

        const glm::mat4 rotationMatrix = glm::mat4_cast(orientation);
        m_ViewMatrix = glm::transpose(rotationMatrix) * glm::translate(glm::mat4(1.0f), -m_Position);
        m_ViewProjectionMatrix = m_ProjectionMatrix * m_ViewMatrix;
    }

    void PerspectiveCamera::RecalculateProjectionMatrix(){
        m_ProjectionMatrix = glm::perspectiveRH_NO(m_VerticalFOVRadians, m_AspectRatio, m_NearClip, m_FarClip);
        m_ViewProjectionMatrix = m_ProjectionMatrix * m_ViewMatrix;
    }
}
