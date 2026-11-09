#include "Sandbox3D.h"

#include <filesystem>
#include <memory>
#include <utility>

#include <glm/glm.hpp>

#include "imgui.h"

#include "Hazel/Renderer/Common/FullscreenTriangle.h"
#include "Hazel/Renderer/Common/Shader.h"
#include "Hazel/Renderer/Common/Texture.h"

#include "Hazel/Renderer/RenderView.h"

#include "Hazel/Renderer/Pipeline/ForwardRenderPipeline.h"
#include "Hazel/Renderer/Pipeline/RenderPass/SkyboxPass.h"
#include "Hazel/Renderer/Pipeline/RenderPass/ToneMappingPass.h"

#include "Hazel/Renderer/Scene/SceneRenderExtractor.h"
#include "Hazel/Renderer/Lighting/EnvironmentMapLoader.h"
#include "Hazel/Renderer/Material/MaterialBinder.h"
#include "Hazel/Scene/Entity.h"
#include "Hazel/Scene/Components/TransformComponent.h"
#include "Hazel/Scene/Components/MeshRendererComponent.h"

namespace
{
    // Sandbox 内的飞行参照物测试 Pass，验证完整相机平移和实体深度。
    // 后续由正式 OpaquePass 替代。
    class FlightReferencePass final : public Hazel::RenderPass
    {
    public:
        const char* GetName() const override { return "FlightReferencePass"; }

        void Execute(const Hazel::RenderPassContext& context) override
        {
            Hazel::RenderState state;
            state.Cull = Hazel::CullMode::None;
            Hazel::RenderCommand::SetRenderState(state);

            for (const auto& item : context.Scene.Meshes)
            {
                Hazel::MaterialBinder::Bind(*item.Material);
                const auto& shader = item.Material->GetMaterial()->GetShader();
                shader->SetMat4("u_ViewProjection", context.View.ViewProjection);
                shader->SetMat4("u_Model", item.WorldMatrix);
                item.MeshResource->GetVertexArray()->Bind();
                Hazel::RenderCommand::DrawIndexed(
                    item.MeshResource->GetVertexArray(), item.MeshResource->GetIndexCount());
            }
        }
    };

    void CreateFlightReferences(Hazel::Scene& scene)
    {
        auto shader = Hazel::Shader::Create("FlightReference", R"(
            #version 330 core
            layout(location = 0) in vec3 a_Position;
            layout(location = 1) in vec3 a_Normal;
            uniform mat4 u_ViewProjection;
            uniform mat4 u_Model;
            out vec3 v_Normal;
            void main()
            {
                v_Normal = transpose(inverse(mat3(u_Model))) * a_Normal;
                gl_Position = u_ViewProjection * u_Model * vec4(a_Position, 1.0);
            }
        )", R"(
            #version 330 core
            in vec3 v_Normal;
            layout(location = 0) out vec4 o_Color;
            uniform vec4 u_Color;
            void main()
            {
                float shade = 0.3 + 0.7 * max(dot(normalize(v_Normal),
                    normalize(vec3(0.4, 1.0, 0.6))), 0.0);
                o_Color = vec4(u_Color.rgb * shade, 1.0);
            }
        )");
        Hazel::ShaderParameters parameters;
        parameters.Set("u_Color", glm::vec4(1.0f));
        auto material = Hazel::CreateRef<Hazel::Material>(
            "FlightReference", shader, std::move(parameters));
        auto cube = Hazel::PrimitiveMesh::CreateCube();

        const auto add = [&](const char* name, glm::vec3 position,
            glm::vec3 scale, glm::vec4 color)
        {
            auto entity = scene.CreateEntity(name);
            auto& transform = entity.AddComponent<Hazel::TransformComponent>();
            transform.Position = position;
            transform.Scale = scale;
            auto& renderer = entity.AddComponent<Hazel::MeshRendererComponent>();
            renderer.MeshResource = cube;
            renderer.Material = Hazel::CreateRef<Hazel::MaterialInstance>(material);
            renderer.Material->Set("u_Color", color);
        };
        add("Flight cube red", { -3.0f, 0.0f, 0.0f }, glm::vec3(1.0f), { 0.8f, 0.1f, 0.05f, 1.0f });
        add("Flight cube green", { 0.0f, 0.0f, -3.0f }, glm::vec3(1.0f), { 0.1f, 0.7f, 0.2f, 1.0f });
        add("Flight cube blue", { 3.0f, 0.0f, -6.0f }, glm::vec3(1.0f), { 0.1f, 0.3f, 0.9f, 1.0f });
        add("Flight platform", { 0.0f, -1.0f, -3.0f }, { 12.0f, 0.1f, 12.0f }, { 0.18f, 0.2f, 0.22f, 1.0f });
    }
}


Sandbox3D::Sandbox3D()
    : Hazel::Layer("Sandbox3D"),
      m_CameraController(
          glm::radians(45.0f),
          1280.0f / 720.0f,
          0.1f,
          1000.0f)
{
}


void Sandbox3D::OnAttach()
{
    // 当前 CMake 使用绝对源文件路径。
    // 从 Sandbox/src/Sandbox3D.cpp 向上定位项目根目录。
    // 这是开发阶段的资源定位方式。
    const std::filesystem::path projectRoot =
        std::filesystem::path(__FILE__)
            .parent_path()
            .parent_path()
            .parent_path();

    const std::filesystem::path engineShaderRoot =
        projectRoot / "Hazel/src/Hazel/Assets/Shaders";

    const std::filesystem::path hdrPath =
        projectRoot /
        "Sandbox/assets/HDR/sunflowers_puresky_8k.hdr";

    // --------------------------------------------------
    // 1. 创建场景，加载 HDR 环境
    // --------------------------------------------------

    m_Scene = std::make_unique<Hazel::Scene>();

    auto& environment = m_Scene->GetEnvironment();

    environment.SkyboxMap =
        Hazel::EnvironmentMapLoader::LoadHDR(hdrPath.string(), 512);

    environment.ShowBackground = true;

    environment.BackgroundIntensity = 1.0f;
    environment.RotationRadians = 0.0f;
    environment.Exposure = 1.0f;

    environment.ClearColor =
        glm::vec4(0.01f, 0.01f, 0.015f, 1.0f);

    // IBL 尚未生成，目前只绘制天空背景。
    environment.EnvironmentLightingEnabled = false;
    environment.AmbientIntensity = 0.0f;

    // --------------------------------------------------
    // 2. 加载内核 Shader
    // --------------------------------------------------

    auto skyboxShader = Hazel::Shader::Create(
        (engineShaderRoot / "Skybox.glsl").string());

    auto toneMappingShader = Hazel::Shader::Create(
        (engineShaderRoot / "ToneMapping.glsl").string());

    // --------------------------------------------------
    // 3. 创建共享全屏几何和 Pass
    // --------------------------------------------------

    auto fullscreenTriangle =
        Hazel::CreateRef<Hazel::FullscreenTriangle>();

    auto skyboxPass =
        std::make_unique<Hazel::SkyboxPass>(
            skyboxShader,
            fullscreenTriangle);

    auto toneMappingPass =
        std::make_unique<Hazel::ToneMappingPass>(
            toneMappingShader,
            fullscreenTriangle);

    // Pass 持有 Shader 和全屏几何的 Ref。
    // 上面的局部 Ref 销毁后，资源仍由 Pass 持有。
    auto pipeline =
        std::make_unique<Hazel::ForwardRenderPipeline>(
            std::move(skyboxPass),
            std::move(toneMappingPass));
    pipeline->AddScenePass(std::make_unique<FlightReferencePass>());
    m_Pipeline = std::move(pipeline);
    CreateFlightReferences(*m_Scene);

    // --------------------------------------------------
    // 4. 初始化相机
    // --------------------------------------------------

    m_CameraController.SetMoveSpeed(5.0f);
    m_CameraController.SetMouseSensitivity(0.003f);

    ResetCamera();

    const auto& window =
        Hazel::Application::Get().GetWindow();

    m_ViewportWidth = window.GetWidth();
    m_ViewportHeight = window.GetHeight();

    if (m_ViewportWidth > 0 && m_ViewportHeight > 0)
    {
        m_CameraController.GetCamera().SetViewportSize(
            m_ViewportWidth,
            m_ViewportHeight);
    }
}


void Sandbox3D::OnDetach()
{
    m_CameraController.SetEnabled(false);

    // 先释放管线拥有的 Pass、Shader 和 Framebuffer。
    m_Pipeline.reset();

    // 清除提取数据中的资源引用。
    m_RenderSceneData.Clear();
    m_TransformSystem.Clear();

    // 场景持有 HDR 环境纹理。
    m_Scene.reset();

    m_ViewportWidth = 0;
    m_ViewportHeight = 0;
}


void Sandbox3D::OnUpdate(Hazel::Timestep timestep)
{
    const auto& window =
        Hazel::Application::Get().GetWindow();

    const uint32_t width = window.GetWidth();
    const uint32_t height = window.GetHeight();

    if (width == 0 || height == 0)
    {
        m_CameraController.SetEnabled(false);
        return;
    }

    // 在这里统一同步相机尺寸，避免零高度进入投影计算。
    if (width != m_ViewportWidth ||
        height != m_ViewportHeight)
    {
        m_ViewportWidth = width;
        m_ViewportHeight = height;

        m_CameraController.GetCamera().SetViewportSize(
            width,
            height);
    }

    // ImGui 操作期间暂停相机输入。
    const auto& io = ImGui::GetIO();

    m_CameraController.SetEnabled(
        !io.WantCaptureMouse &&
        !io.WantCaptureKeyboard);

    m_CameraController.OnUpdate(timestep);

    // 所有场景变换更新完成后，再提取渲染数据。
    // 天空只使用相机旋转，场景参照物使用完整的视图矩阵。
    m_TransformSystem.Update(*m_Scene);

    Hazel::SceneRenderExtractor::Extract(
        *m_Scene,
        m_TransformSystem,
        m_RenderSceneData);

    const Hazel::RenderView view =
        Hazel::RenderView::FromCamera(
            m_CameraController.GetCamera(),
            width,
            height);

    // output 为空，管线将最终颜色输出到窗口。
    // HDR 目标、清除、Pass 顺序和视口均由管线管理。
    m_Pipeline->Render(
        m_RenderSceneData,
        view);
}


void Sandbox3D::OnImGuiRender()
{
    if (ImGui::Begin("HDR Skybox"))
    {
        auto& environment = m_Scene->GetEnvironment();

        ImGui::TextUnformatted(
            "valley_of_desolation_4k.hdr (Cubemap)");

        ImGui::Separator();

        ImGui::Checkbox(
            "Show background",
            &environment.ShowBackground);

        ImGui::SliderFloat(
            "Background intensity",
            &environment.BackgroundIntensity,
            0.0f,
            5.0f,
            "%.2f");

        ImGui::SliderAngle(
            "Environment rotation",
            &environment.RotationRadians,
            -180.0f,
            180.0f);

        // 当前 Shader 的曝光参数是线性倍率。
        // 使用对数滑块，方便观察夜景 HDR。
        ImGui::SliderFloat(
            "Exposure",
            &environment.Exposure,
            0.01f,
            100.0f,
            "%.3f",
            ImGuiSliderFlags_Logarithmic);

        ImGui::ColorEdit4(
            "Clear color",
            &environment.ClearColor.r);

        ImGui::Separator();

        if (ImGui::Button("Reset camera"))
        {
            ResetCamera();
        }

        ImGui::SameLine();

        if (ImGui::Button("Reset environment"))
        {
            environment.ShowBackground = true;
            environment.BackgroundIntensity = 1.0f;
            environment.RotationRadians = 0.0f;
            environment.Exposure = 1.0f;
        }

        const auto& camera =
            m_CameraController.GetCamera();

        const glm::vec3 position = camera.GetPosition();

        const glm::vec3 rotationDegrees =
            glm::degrees(camera.GetRotation());

        ImGui::Text(
            "Position: %.2f, %.2f, %.2f",
            position.x,
            position.y,
            position.z);

        ImGui::Text(
            "Pitch / Yaw: %.1f / %.1f",
            rotationDegrees.x,
            rotationDegrees.y);

        ImGui::Text(
            "Viewport: %u x %u",
            m_ViewportWidth,
            m_ViewportHeight);

        ImGui::Separator();

        ImGui::TextUnformatted(
            "Hold RMB and move mouse to look around.");

        ImGui::TextUnformatted(
            "WASD: move   Q/E: down/up   Shift: boost");

        ImGui::TextUnformatted(
            "RMB + wheel: change movement speed");

        ImGui::TextWrapped(
            "Fly around the cubes and platform. "
            "The distant sky follows camera rotation only.");
    }

    ImGui::End();
}


void Sandbox3D::OnEvent(Hazel::Event& event)
{
    // 相机尺寸在 OnUpdate 中统一同步。
    // 不向控制器转发 Resize，避免重复更新和零高度计算。
    if (event.GetEventType() ==
        Hazel::EventType::WindowResize)
    {
        return;
    }

    const auto& io = ImGui::GetIO();

    if (io.WantCaptureMouse &&
        event.IsInCategory(Hazel::EventCategoryMouse))
    {
        return;
    }

    if (io.WantCaptureKeyboard &&
        event.IsInCategory(Hazel::EventCategoryKeyboard))
    {
        return;
    }

    // 当前控制器的滚轮处理没有检查 m_Enabled，
    // 因此在 Sandbox 入口处阻止禁用期间的输入事件。
    if (!m_CameraController.IsEnabled())
    {
        return;
    }

    m_CameraController.OnEvent(event);
}


void Sandbox3D::ResetCamera()
{
    auto& camera = m_CameraController.GetCamera();

    camera.SetPosition(glm::vec3(0.0f, 1.0f, 6.0f));
    camera.SetRotation(glm::vec3(0.0f));
}
