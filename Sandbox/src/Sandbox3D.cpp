#include "Sandbox3D.h"

#include <cstdint>

#include <glm/gtc/matrix_transform.hpp>

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
    // ---------------------------------------------------------
    // 1. 设置初始相机
    // ---------------------------------------------------------
    auto& camera = m_CameraController.GetCamera();

    camera.SetPosition({ 0.0f, 0.0f, 5.0f });
    camera.SetRotation({ 0.0f, 0.0f, 0.0f });

    auto& window = Hazel::Application::Get().GetWindow();

    camera.SetViewportSize(
        window.GetWidth(),
        window.GetHeight());

    // ---------------------------------------------------------
    // 2. 创建立方体顶点
    // 每个顶点：Position.xyz + Color.rgb
    // ---------------------------------------------------------
    float vertices[] = {
        // Position                // Color
        -0.5f, -0.5f, -0.5f,       0.2f, 0.2f, 0.8f, // 0
         0.5f, -0.5f, -0.5f,       0.8f, 0.2f, 0.2f, // 1
         0.5f,  0.5f, -0.5f,       0.8f, 0.8f, 0.2f, // 2
        -0.5f,  0.5f, -0.5f,       0.2f, 0.8f, 0.2f, // 3

        -0.5f, -0.5f,  0.5f,       0.2f, 0.8f, 0.8f, // 4
         0.5f, -0.5f,  0.5f,       0.8f, 0.2f, 0.8f, // 5
         0.5f,  0.5f,  0.5f,       1.0f, 1.0f, 1.0f, // 6
        -0.5f,  0.5f,  0.5f,       0.4f, 0.8f, 0.4f  // 7
    };

    // 六个面，每个面两个三角形。
    // 从立方体外部观察，各面的绕序均为逆时针。
    uint32_t indices[] = {
        // 后面 -Z
        0, 2, 1,
        2, 0, 3,

        // 前面 +Z
        4, 5, 6,
        6, 7, 4,

        // 左面 -X
        0, 4, 7,
        7, 3, 0,

        // 右面 +X
        1, 2, 6,
        6, 5, 1,

        // 下面 -Y
        0, 1, 5,
        5, 4, 0,

        // 上面 +Y
        3, 7, 6,
        6, 2, 3
    };

    // ---------------------------------------------------------
    // 3. 创建 VAO / VBO / IBO
    // ---------------------------------------------------------
    m_CubeVertexArray = Hazel::VertexArray::Create();

    auto vertexBuffer = Hazel::VertexBuffer::Create(
        vertices,
        static_cast<uint32_t>(sizeof(vertices)));

    vertexBuffer->SetLayout({
        { Hazel::ShaderDataType::Float3, "a_Position" },
        { Hazel::ShaderDataType::Float3, "a_Color" }
    });

    m_CubeVertexArray->AddVertexBuffer(vertexBuffer);

    const uint32_t indexCount =
        static_cast<uint32_t>(
            sizeof(indices) / sizeof(indices[0]));

    auto indexBuffer = Hazel::IndexBuffer::Create(
        indices,
        indexCount);

    m_CubeVertexArray->SetIndexBuffer(indexBuffer);

    // ---------------------------------------------------------
    // 4. 创建 Shader
    // ---------------------------------------------------------
    const std::string vertexSource = R"(
        #version 330 core

        layout(location = 0) in vec3 a_Position;
        layout(location = 1) in vec3 a_Color;

        uniform mat4 u_ViewProjection;
        uniform mat4 u_Transform;

        out vec3 v_Color;

        void main()
        {
            v_Color = a_Color;

            gl_Position =
                u_ViewProjection *
                u_Transform *
                vec4(a_Position, 1.0);
        }
    )";

    const std::string fragmentSource = R"(
        #version 330 core

        layout(location = 0) out vec4 o_Color;

        in vec3 v_Color;

        uniform vec4 u_Tint;

        void main()
        {
            o_Color = vec4(v_Color, 1.0) * u_Tint;
        }
    )";

    m_CubeShader = Hazel::Shader::Create(
        "Sandbox3D_Cube",
        vertexSource,
        fragmentSource);
}

void Sandbox3D::OnDetach()
{
    m_CubeShader.reset();
    m_CubeVertexArray.reset();
}

void Sandbox3D::OnUpdate(Hazel::Timestep timestep)
{
    // ---------------------------------------------------------
    // 1. 更新相机
    // ---------------------------------------------------------
    m_CameraController.OnUpdate(timestep);

    // ---------------------------------------------------------
    // 2. 清屏
    // 你的 OpenGLRendererAPI 已启用深度测试，
    // Clear() 也会清除颜色和深度缓冲。
    // ---------------------------------------------------------
    Hazel::RenderCommand::SetClearColor({
        0.08f, 0.09f, 0.12f, 1.0f
    });

    Hazel::RenderCommand::Clear();

    // ---------------------------------------------------------
    // 3. 上传本次视图共用的数据
    // ---------------------------------------------------------
    auto& camera = m_CameraController.GetCamera();

    m_CubeShader->Bind();

    m_CubeShader->SetMat4(
        "u_ViewProjection",
        camera.GetViewProjectionMatrix());

    m_CubeVertexArray->Bind();

    // ---------------------------------------------------------
    // 4. 同一份几何，使用不同的 Transform 绘制三个实例
    // ---------------------------------------------------------
    const glm::vec3 positions[] = {
        { -1.3f, 0.0f,  0.0f },
        {  0.0f, 0.0f, -1.5f },
        {  1.3f, 0.0f,  0.0f }
    };

    const glm::vec4 tints[] = {
        { 1.0f, 0.7f, 0.7f, 1.0f },
        { 0.7f, 1.0f, 0.7f, 1.0f },
        { 0.7f, 0.7f, 1.0f, 1.0f }
    };

    for (uint32_t i = 0; i < 3; ++i)
    {
        const glm::mat4 transform =
            glm::translate(glm::mat4(1.0f), positions[i]) *
            glm::scale(glm::mat4(1.0f), glm::vec3(0.8f));

        m_CubeShader->SetMat4(
            "u_Transform",
            transform);

        m_CubeShader->SetFloat4(
            "u_Tint",
            tints[i]);

        Hazel::RenderCommand::DrawIndexed(
            m_CubeVertexArray,
            36);
    }
}

void Sandbox3D::OnEvent(Hazel::Event& event)
{
    m_CameraController.OnEvent(event);
}