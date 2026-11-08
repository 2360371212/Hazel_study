#pragma once

#include <unordered_map>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include "Hazel/Core/Core.h"
#include "Hazel/Core/UUID.h"
#include "Hazel/Scene/Scene.h"

namespace Hazel
{

    struct WorldTransformData
    {
        glm::mat4 Matrix{ 1.0f };
        glm::vec3 Position{ 0.0f };
        glm::quat Rotation{ 1.0f, 0.0f, 0.0f, 0.0f };

        glm::vec3 GetForward() const
        {
            return Rotation * glm::vec3(0.0f, 0.0f, -1.0f);
        }

        glm::vec3 GetRight() const
        {
            return Rotation * glm::vec3(1.0f, 0.0f, 0.0f);
        }

        glm::vec3 GetUp() const
        {
            return Rotation * glm::vec3(0.0f, 1.0f, 0.0f);
        }
    };

    class HAZEL_API TransformSystem
    {
        public:
            // 在本帧所有变换修改完成后、渲染提交之前调用。
            void Update(const Scene& scene);

            const WorldTransformData& GetWorldTransform(UUID id) const;

            void Clear();

        private:
            // 再保存一份组建的状况
            std::unordered_map<UUID, WorldTransformData> m_WorldTransforms;
    };
}