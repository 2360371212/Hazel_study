#pragma once
#include <memory>
#include "Hazel/Core/Core.h"
#include "Hazel/Renderer/Buffer.h"

namespace Hazel{
    class VertexArray{
        public:
            virtual ~VertexArray(){}

            virtual void Bind() const = 0;
            virtual void UnBind() const = 0;

            virtual const void AddVertexBuffer(const Ref<VertexBuffer> vertexBuffer) = 0;
            virtual const void SetIndexBuffer(const Ref<IndexBuffer> indexBuffer) = 0;

            virtual const std::vector<Ref<VertexBuffer>> GetVertexBuffers() const = 0;
            virtual const Ref<IndexBuffer> GetIndexBuffer() const = 0;

            // virtual void SetLayout(const BufferLayout& layout) const = 0;
            static Ref<VertexArray> Create();

    };
}