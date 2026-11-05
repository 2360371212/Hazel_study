#pragma once

#include <cstdint>
#include <vector>

#include <glm/glm.hpp>

namespace Hazel
{
    struct MeshVertex
    {
        glm::vec3 Position{ 0.0f };
        glm::vec3 Normal{ 0.0f, 1.0f, 0.0f };
        glm::vec2 TexCoord{ 0.0f };
    };

    // 轴对齐包围盒
    struct AABB
    {
        glm::vec3 Min{ 0.0f };
        glm::vec3 Max{ 0.0f };
    };

    struct MeshData
    {
        std::vector<MeshVertex> Vertices;
        std::vector<uint32_t> Indices;
        // 物体的包围盒
        AABB Bounds;

        bool IsIndexed() const {
            return !Indices.empty();
        }
    };

    AABB CalculateBounds(const std::vector<MeshVertex>& vertices);
}