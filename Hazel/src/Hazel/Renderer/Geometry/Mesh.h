#pragma once

#include "Hazel/Core/Core.h"
#include "Hazel/Renderer/Common/VertexArray.h"
#include "MeshTypes.h"

namespace Hazel
{
    class Mesh
    {
    public:
        Mesh(const MeshData& data);
        virtual ~Mesh() = default;

        static Ref<Mesh> Create(const MeshData& data);

        const Ref<VertexArray>& GetVertexArray() const
        {
            return m_VertexArray;
        }

        uint32_t GetVertexCount() const
        {
            return m_VertexCount;
        }

        uint32_t GetIndexCount() const
        {
            return m_IndexCount;
        }

        const AABB& GetBounds() const
        {
            return m_Bounds;
        }

    private:
        Ref<VertexArray> m_VertexArray;

        uint32_t m_VertexCount = 0;
        uint32_t m_IndexCount = 0;
        
        AABB m_Bounds;
    };
}