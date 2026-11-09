#include "hzpch.h"
#include "FullscreenTriangle.h"

#include "Hazel/Renderer/Common/Buffer.h"
#include "Hazel/Renderer/Common/RenderCommand.h"

namespace Hazel
{
    FullscreenTriangle::FullscreenTriangle()
    {
        // 一个大三角形覆盖整个 NDC 屏幕区域。
        float vertices[] = {
            -1.0f, -1.0f,
             3.0f, -1.0f,
            -1.0f,  3.0f
        };

        uint32_t indices[] = {
            0, 1, 2
        };

        auto vertexBuffer = VertexBuffer::Create(vertices, static_cast<uint32_t>(sizeof(vertices)));

        vertexBuffer->SetLayout({{ShaderDataType::Float2, "a_Position"}});
        auto indexBuffer = IndexBuffer::Create(indices, 3);

        m_VertexArray = VertexArray::Create();

        m_VertexArray->AddVertexBuffer(vertexBuffer);
        m_VertexArray->SetIndexBuffer(indexBuffer);
    }

    void FullscreenTriangle::Draw() const
    {
        m_VertexArray->Bind();
        RenderCommand::DrawIndexed(m_VertexArray, 3);
    }
}