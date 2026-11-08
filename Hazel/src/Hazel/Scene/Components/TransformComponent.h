#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

namespace Hazel
{
    struct TransformComponent
    {
        glm::vec3 Position{ 0.0f };

        // 欧拉角，单位：弧度。
        glm::vec3 Rotation{ 0.0f };

        glm::vec3 Scale{ 1.0f };

        glm::quat GetRotationQuaternion() const
        {
            return glm::quat(Rotation);
        }

        glm::mat4 GetLocalMatrix() const
        {
            const glm::mat4 translation = glm::translate(glm::mat4(1.0f), Position);
            const glm::mat4 rotation = glm::mat4_cast(GetRotationQuaternion());
            const glm::mat4 scale = glm::scale(glm::mat4(1.0f), Scale);

            return translation * rotation * scale;
        }

        // 约定实体局部坐标的 -Z 为前方。
        glm::vec3 GetForward() const
        {
            return GetRotationQuaternion() * glm::vec3(0.0f, 0.0f, -1.0f);
        }

        glm::vec3 GetRight() const
        {
            return GetRotationQuaternion() * glm::vec3(1.0f, 0.0f, 0.0f);
        }

        glm::vec3 GetUp() const
        {
            return GetRotationQuaternion() * glm::vec3(0.0f, 1.0f, 0.0f);
        }
    };
}