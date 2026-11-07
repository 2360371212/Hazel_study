#include "hzpch.h"
#include "PrimitiveMesh.h"

#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>


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

    Ref<Mesh> PrimitiveMesh::CreateSphere(float radius, uint32_t longitudeSegments, uint32_t latitudeSegments)
    {
        MeshData data;
        const uint32_t rowVertexCount = longitudeSegments + 1;

        // 创建对应的容量，但是不填充数据
        data.Vertices.reserve(rowVertexCount * (latitudeSegments + 1));
        data.Indices.reserve(longitudeSegments * (latitudeSegments - 1) * 6);

        // 顶点。
        for (uint32_t latitude = 0; latitude <= latitudeSegments; ++latitude)
        {
            const float v = static_cast<float>(latitude) / static_cast<float>(latitudeSegments);
            const float theta = v * glm::pi<float>();

            const float sinTheta = std::sin(theta);
            const float cosTheta = std::cos(theta);

            for (uint32_t longitude = 0; longitude <= longitudeSegments; ++longitude)
            {
                const float u = static_cast<float>(longitude) / static_cast<float>(longitudeSegments);
                const float phi = u * glm::two_pi<float>();
                const glm::vec3 normal{sinTheta * std::cos(phi), cosTheta, sinTheta * std::sin(phi)};
                data.Vertices.push_back({normal * radius, normal, { u, 1.0f - v }});
            }
        }

        // 索引：从球体外部观察为逆时针。
        for (uint32_t latitude = 0; latitude < latitudeSegments; ++latitude)
        {
            for (uint32_t longitude = 0; longitude < longitudeSegments; ++longitude)
            {
                const uint32_t a = latitude * rowVertexCount + longitude;
                const uint32_t b = a + rowVertexCount;

                // 北极处跳过退化三角形。
                if (latitude > 0)
                {
                    data.Indices.insert(data.Indices.end(), {a, a + 1, b});
                }

                // 南极处跳过退化三角形。
                if (latitude + 1 < latitudeSegments)
                {
                    data.Indices.insert(data.Indices.end(), {a + 1, b + 1, b});
                }
            }
        }
        data.Bounds.Min = glm::vec3(-radius);
        data.Bounds.Max = glm::vec3(radius);

        return Mesh::Create(data);
    }
}