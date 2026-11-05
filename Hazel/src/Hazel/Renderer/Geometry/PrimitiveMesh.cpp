#include "hzpch.h"
#include "PrimitiveMesh.h"

#include <glm/glm.hpp>

namespace Hazel
{
    namespace
    {
        void AddFace(MeshData& data, const glm::vec3& p0, const glm::vec3& p1, const glm::vec3& p2, const glm::vec3& p3, const glm::vec3& normal)
        {
            const uint32_t baseIndex = static_cast<uint32_t>(data.Vertices.size());

            data.Vertices.push_back({p0, normal, { 0.0f, 0.0f }});
            data.Vertices.push_back({p1, normal, { 1.0f, 0.0f }});
            data.Vertices.push_back({p2, normal, { 1.0f, 1.0f }});
            data.Vertices.push_back({p3, normal, { 0.0f, 1.0f }});

            data.Indices.insert(data.Indices.end(),
                {
                    baseIndex + 0,
                    baseIndex + 1,
                    baseIndex + 2,

                    baseIndex + 2,
                    baseIndex + 3,
                    baseIndex + 0
                });
        }
    }

    Ref<Mesh> PrimitiveMesh::CreateCube(float size)
    {
        MeshData data;

        const float halfSize =
            size * 0.5f;

        // 前面 +Z
        AddFace(
            data,
            { -halfSize, -halfSize,  halfSize },
            {  halfSize, -halfSize,  halfSize },
            {  halfSize,  halfSize,  halfSize },
            { -halfSize,  halfSize,  halfSize },
            { 0.0f, 0.0f, 1.0f });

        // 后面 -Z
        AddFace(
            data,
            {  halfSize, -halfSize, -halfSize },
            { -halfSize, -halfSize, -halfSize },
            { -halfSize,  halfSize, -halfSize },
            {  halfSize,  halfSize, -halfSize },
            { 0.0f, 0.0f, -1.0f });

        // 左面 -X
        AddFace(
            data,
            { -halfSize, -halfSize, -halfSize },
            { -halfSize, -halfSize,  halfSize },
            { -halfSize,  halfSize,  halfSize },
            { -halfSize,  halfSize, -halfSize },
            { -1.0f, 0.0f, 0.0f });

        // 右面 +X
        AddFace(
            data,
            {  halfSize, -halfSize,  halfSize },
            {  halfSize, -halfSize, -halfSize },
            {  halfSize,  halfSize, -halfSize },
            {  halfSize,  halfSize,  halfSize },
            { 1.0f, 0.0f, 0.0f });

        // 顶面 +Y
        AddFace(
            data,
            { -halfSize,  halfSize,  halfSize },
            {  halfSize,  halfSize,  halfSize },
            {  halfSize,  halfSize, -halfSize },
            { -halfSize,  halfSize, -halfSize },
            { 0.0f, 1.0f, 0.0f });

        // 底面 -Y
        AddFace(
            data,
            { -halfSize, -halfSize, -halfSize },
            {  halfSize, -halfSize, -halfSize },
            {  halfSize, -halfSize,  halfSize },
            { -halfSize, -halfSize,  halfSize },
            { 0.0f, -1.0f, 0.0f });

        data.Bounds = CalculateBounds(data.Vertices);

        return Mesh::Create(data);
    }

    Ref<Mesh> PrimitiveMesh::CreatePlane(float width, float depth)
    {
        MeshData data;

        const float halfWidth = width * 0.5f;

        const float halfDepth = depth * 0.5f;

        data.Vertices = {
            {
                { -halfWidth, 0.0f, -halfDepth },
                {  0.0f, 1.0f,  0.0f },
                {  0.0f, 0.0f }
            },
            {
                {  halfWidth, 0.0f, -halfDepth },
                {  0.0f, 1.0f,  0.0f },
                {  1.0f, 0.0f }
            },
            {
                {  halfWidth, 0.0f,  halfDepth },
                {  0.0f, 1.0f,  0.0f },
                {  1.0f, 1.0f }
            },
            {
                { -halfWidth, 0.0f,  halfDepth },
                {  0.0f, 1.0f,  0.0f },
                {  0.0f, 1.0f }
            }
        };

        data.Indices = {0, 1, 2, 2, 3, 0};
        data.Bounds = CalculateBounds(data.Vertices);

        return Mesh::Create(data);
    }
}