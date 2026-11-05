#include "hzpch.h"
#include "Mesh.h"

#include "Hazel/Core/Log.h"
#include "Hazel/Renderer/Common/Buffer.h"

#include <limits>

namespace Hazel
{
    AABB CalculateBounds(const std::vector<MeshVertex>& vertices)
    {
        AABB bounds;
        bounds.Min = vertices[0].Position;
        bounds.Max = vertices[0].Position;
        for (const MeshVertex& vertex : vertices)
        {
            bounds.Min = glm::min(bounds.Min, vertex.Position);
            bounds.Max = glm::max(bounds.Max, vertex.Position);
        }
        return bounds;
    }

    Ref<Mesh> Mesh::Create(const MeshData& data)
    {
        return CreateRef<Mesh>(data);
    }

    Mesh::Mesh(const MeshData& data)
    {
        // HZ_CORE_ASSERT(!data.Vertices.empty(), "Mesh must contain at least one vertex");
        // HZ_CORE_ASSERT(!data.Indices.empty(), "Current Mesh implementation requires indexed geometry");

        if (data.Vertices.empty() || data.Indices.empty())
            return;

        m_VertexCount = static_cast<uint32_t>(data.Vertices.size());

        m_IndexCount = static_cast<uint32_t>(data.Indices.size());

        // 当前 VertexBuffer::Create 接口接收 float*，
        // MeshVertex 的内存布局是连续的，因此可以直接上传。
        auto vertexBuffer = VertexBuffer::Create(data.Vertices);

        vertexBuffer->SetLayout({
            { ShaderDataType::Float3, "a_Position" },
            { ShaderDataType::Float3, "a_Normal" },
            { ShaderDataType::Float2, "a_TexCoord" }
        });

        auto indexBuffer = IndexBuffer::Create(const_cast<uint32_t*>(data.Indices.data()), m_IndexCount);

        m_VertexArray = VertexArray::Create();

        m_VertexArray->AddVertexBuffer(vertexBuffer);
        m_VertexArray->SetIndexBuffer(indexBuffer);

        // // 如果调用方没有预先设置包围盒，则自动计算。
        // if (data.Bounds.Min == glm::vec3(0.0f) && data.Bounds.Max == glm::vec3(0.0f))
        // {
        //     m_Bounds = CalculateBounds(data.Vertices);
        // }
        // else
        // {
        //     m_Bounds = data.Bounds;
        // }
    }
}