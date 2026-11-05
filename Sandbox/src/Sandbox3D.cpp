#include "Sandbox3D.h"

#include <algorithm>
#include <cstdint>
#include <limits>


#include <glm/gtc/matrix_transform.hpp>

namespace
{
    Hazel::AABB TransformAABB(
        const Hazel::AABB& localBounds,
        const glm::mat4& transform)
    {
        const glm::vec3 corners[8] = {
            { localBounds.Min.x, localBounds.Min.y, localBounds.Min.z },
            { localBounds.Max.x, localBounds.Min.y, localBounds.Min.z },
            { localBounds.Max.x, localBounds.Max.y, localBounds.Min.z },
            { localBounds.Min.x, localBounds.Max.y, localBounds.Min.z },

            { localBounds.Min.x, localBounds.Min.y, localBounds.Max.z },
            { localBounds.Max.x, localBounds.Min.y, localBounds.Max.z },
            { localBounds.Max.x, localBounds.Max.y, localBounds.Max.z },
            { localBounds.Min.x, localBounds.Max.y, localBounds.Max.z }
        };

        Hazel::AABB worldBounds;

        worldBounds.Min =
            glm::vec3(std::numeric_limits<float>::max());

        worldBounds.Max =
            glm::vec3(std::numeric_limits<float>::lowest());

        for (const glm::vec3& corner : corners)
        {
            const glm::vec3 worldPosition =
                glm::vec3(
                    transform *
                    glm::vec4(corner, 1.0f));

            worldBounds.Min =
                glm::min(
                    worldBounds.Min,
                    worldPosition);

            worldBounds.Max =
                glm::max(
                    worldBounds.Max,
                    worldPosition);
        }

        return worldBounds;
    }
}

Sandbox3D::Sandbox3D()
    : Hazel::Layer("Sandbox3D"),
      m_CameraController(
          glm::radians(45.0f),
          1280.0f / 720.0f,
          0.1f,
          100.0f)
{
}

void Sandbox3D::OnAttach()
{
    auto& camera =
        m_CameraController.GetCamera();

    camera.SetPosition({
        0.0f,
        0.0f,
        5.0f
    });

    camera.SetRotation({
        0.0f,
        0.0f,
        0.0f
    });

    auto& window =
        Hazel::Application::Get().GetWindow();

    if (window.GetWidth() != 0 &&
        window.GetHeight() != 0)
    {
        camera.SetViewportSize(
            window.GetWidth(),
            window.GetHeight());
    }

    // 创建几何 Mesh。
    m_Cube =
        Hazel::PrimitiveMesh::CreateCube(1.0f);

    // 创建普通立方体 Shader。
    const std::string cubeVertexSource = R"(
        #version 330 core

        layout(location = 0) in vec3 a_Position;
        layout(location = 1) in vec3 a_Normal;
        layout(location = 2) in vec2 a_TexCoord;

        uniform mat4 u_ViewProjection;
        uniform mat4 u_Transform;

        out vec3 v_Normal;
        out vec2 v_TexCoord;

        void main()
        {
            v_Normal = a_Normal;
            v_TexCoord = a_TexCoord;

            gl_Position =
                u_ViewProjection *
                u_Transform *
                vec4(a_Position, 1.0);
        }
    )";

    const std::string cubeFragmentSource = R"(
        #version 330 core

        layout(location = 0) out vec4 o_Color;

        in vec3 v_Normal;
        in vec2 v_TexCoord;

        uniform vec4 u_Tint;

        void main()
        {
            o_Color = u_Tint;
        }
    )";

    m_CubeShader =
        Hazel::Shader::Create(
            "Sandbox3D_Cube",
            cubeVertexSource,
            cubeFragmentSource);

    CreateBoundsDebugGeometry();
}

void Sandbox3D::CreateBoundsDebugGeometry()
{
    // 单位立方体。
    // 实际包围盒的位置和大小通过 u_Transform 控制。
    float vertices[] = {
        -0.5f, -0.5f, -0.5f,
         0.5f, -0.5f, -0.5f,
         0.5f,  0.5f, -0.5f,
        -0.5f,  0.5f, -0.5f,

        -0.5f, -0.5f,  0.5f,
         0.5f, -0.5f,  0.5f,
         0.5f,  0.5f,  0.5f,
        -0.5f,  0.5f,  0.5f
    };

    uint32_t indices[] = {
        // 后面
        0, 1, 2,
        2, 3, 0,

        // 前面
        4, 5, 6,
        6, 7, 4,

        // 左面
        0, 4, 7,
        7, 3, 0,

        // 右面
        1, 2, 6,
        6, 5, 1,

        // 底面
        0, 1, 5,
        5, 4, 0,

        // 顶面
        3, 7, 6,
        6, 2, 3
    };

    m_BoundsVertexArray =
        Hazel::VertexArray::Create();

    auto vertexBuffer =
        Hazel::VertexBuffer::Create(
            vertices,
            static_cast<uint32_t>(
                sizeof(vertices)));

    vertexBuffer->SetLayout({
        {
            Hazel::ShaderDataType::Float3,
            "a_Position"
        }
    });

    m_BoundsVertexArray->AddVertexBuffer(
        vertexBuffer);

    auto indexBuffer =
        Hazel::IndexBuffer::Create(
            indices,
            static_cast<uint32_t>(
                sizeof(indices) /
                sizeof(indices[0])));

    m_BoundsVertexArray->SetIndexBuffer(
        indexBuffer);

    const std::string vertexSource = R"(
        #version 330 core

        layout(location = 0) in vec3 a_Position;

        uniform mat4 u_ViewProjection;
        uniform mat4 u_Transform;

        void main()
        {
            gl_Position =
                u_ViewProjection *
                u_Transform *
                vec4(a_Position, 1.0);
        }
    )";

    const std::string fragmentSource = R"(
        #version 330 core

        layout(location = 0) out vec4 o_Color;

        uniform vec4 u_Color;

        void main()
        {
            o_Color = u_Color;
        }
    )";

    m_BoundsShader =
        Hazel::Shader::Create(
            "AABBDebugShader",
            vertexSource,
            fragmentSource);
}

void Sandbox3D::DrawBounds(
    const Hazel::AABB& localBounds,
    const glm::mat4& transform)
{
    const Hazel::AABB worldBounds =
        TransformAABB(
            localBounds,
            transform);

    const glm::vec3 center =
        (worldBounds.Min +
         worldBounds.Max) *
        0.5f;

    const glm::vec3 size =
        worldBounds.Max -
        worldBounds.Min;

    if (glm::dot(size, size) <= 0.000001f)
        return;

    const glm::mat4 boundsTransform =
        glm::translate(
            glm::mat4(1.0f),
            center) *
        glm::scale(
            glm::mat4(1.0f),
            size);

    auto& camera =
        m_CameraController.GetCamera();

    m_BoundsShader->Bind();

    m_BoundsShader->SetMat4(
        "u_ViewProjection",
        camera.GetViewProjectionMatrix());

    m_BoundsShader->SetMat4(
        "u_Transform",
        boundsTransform);

    m_BoundsShader->SetFloat4(
        "u_Color",
        glm::vec4(
            1.0f,
            0.05f,
            0.02f,
            1.0f));

    m_BoundsVertexArray->Bind();

    // 调试阶段关闭深度测试，
    // 让包围盒始终显示在物体外面。
    // glDisable(GL_DEPTH_TEST);

    // glPolygonMode(
    //     GL_FRONT_AND_BACK,
    //     GL_LINE);

    // glLineWidth(2.0f);

    // Hazel::RenderCommand::DrawIndexed(
    //     m_BoundsVertexArray,
    //     36);

    // // 恢复默认 OpenGL 状态。
    // glPolygonMode(
    //     GL_FRONT_AND_BACK,
    //     GL_FILL);

    // glEnable(GL_DEPTH_TEST);
}

void Sandbox3D::OnDetach()
{
    m_BoundsShader.reset();
    m_BoundsVertexArray.reset();

    m_CubeShader.reset();
    m_Cube.reset();
}

void Sandbox3D::OnUpdate(
    Hazel::Timestep timestep)
{
    m_CameraController.OnUpdate(
        timestep);

    Hazel::RenderCommand::SetClearColor({
        0.08f,
        0.09f,
        0.12f,
        1.0f
    });

    Hazel::RenderCommand::Clear();

    auto& camera =
        m_CameraController.GetCamera();

    const glm::vec3 positions[] = {
        { -1.3f, 0.0f,  0.0f },
        {  0.0f, 0.0f, -1.5f },
        {  1.3f, 0.0f,  0.0f }
    };

    const glm::vec4 colors[] = {
        { 1.0f, 0.35f, 0.35f, 1.0f },
        { 0.35f, 1.0f, 0.35f, 1.0f },
        { 0.35f, 0.35f, 1.0f, 1.0f }
    };

    static float rotation = 0.0f;

    rotation +=
        static_cast<float>(timestep) *
        glm::radians(25.0f);

    for (uint32_t i = 0; i < 3; ++i)
    {
        glm::mat4 transform =
            glm::translate(
                glm::mat4(1.0f),
                positions[i]);

        if (i == 1)
        {
            transform =
                transform *
                glm::rotate(
                    glm::mat4(1.0f),
                    rotation,
                    glm::vec3(
                        0.0f,
                        1.0f,
                        0.0f));
        }

        if (i == 2)
        {
            transform =
                transform *
                glm::rotate(
                    glm::mat4(1.0f),
                    rotation,
                    glm::vec3(
                        1.0f,
                        1.0f,
                        0.0f));
        }

        transform =
            transform *
            glm::scale(
                glm::mat4(1.0f),
                glm::vec3(0.8f));

        // 绘制正常 Mesh。
        m_CubeShader->Bind();

        m_CubeShader->SetMat4(
            "u_ViewProjection",
            camera.GetViewProjectionMatrix());

        m_CubeShader->SetMat4(
            "u_Transform",
            transform);

        m_CubeShader->SetFloat4(
            "u_Tint",
            colors[i]);

        const auto& vertexArray =
            m_Cube->GetVertexArray();

        vertexArray->Bind();

        Hazel::RenderCommand::DrawIndexed(
            vertexArray,
            m_Cube->GetIndexCount());

        // 绘制该物体的世界空间 AABB。
        DrawBounds(
            m_Cube->GetBounds(),
            transform);
    }
}

void Sandbox3D::OnEvent(
    Hazel::Event& event)
{
    m_CameraController.OnEvent(
        event);
}