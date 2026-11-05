#include "Sandbox3D.h"
#include "Hazel/Events/ApplicationEvent.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <exception>
#include <filesystem>
#include <fstream>
#include <functional>
#include <memory>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <glad/glad.h>
#include <imgui.h>

namespace
{
    // All generated texels contain linear RGB values. Row zero is the
    // bottom of the panorama, matching texture UVs without an image flip.
    glm::vec3 PanoramaDirection(float u, float v)
    {
        const float azimuth = (u - 0.5f) * glm::two_pi<float>();
        const float elevation = (v - 0.5f) * glm::pi<float>();
        const float radius = std::cos(elevation);

        return {
            radius * std::cos(azimuth),
            std::sin(elevation),
            radius * std::sin(azimuth)
        };
    }

    glm::vec3 SampleDefaultEnvironment(const glm::vec3& direction)
    {
        const glm::vec3 horizon{ 0.65f, 0.72f, 0.8f };
        const glm::vec3 zenith{ 0.08f, 0.24f, 0.55f };
        const glm::vec3 ground{ 0.12f, 0.10f, 0.08f };

        glm::vec3 color = direction.y >= 0.0f
            ? glm::mix(horizon, zenith, std::sqrt(direction.y))
            : glm::mix(horizon, ground, std::sqrt(-direction.y));

        const glm::vec3 sunDirection =
            glm::normalize(glm::vec3(0.4f, 0.8f, 0.5f));

        const float sun = std::pow(
            std::max(glm::dot(direction, sunDirection), 0.0f),
            512.0f);

        return color + glm::vec3(0.65f, 0.5f, 0.3f) * sun;
    }

    glm::vec3 DiffuseEnvironment(const glm::vec3& normal)
    {
        const glm::vec3 up = std::abs(normal.y) < 0.999f
            ? glm::vec3(0.0f, 1.0f, 0.0f)
            : glm::vec3(1.0f, 0.0f, 0.0f);

        const glm::vec3 tangent =
            glm::normalize(glm::cross(up, normal));
        const glm::vec3 bitangent = glm::cross(normal, tangent);

        constexpr uint32_t sampleCount = 256;
        glm::vec3 average{ 0.0f };

        // Cosine-weighted hemisphere sampling estimates irradiance / pi.
        // This work runs once during environment creation, not per frame.
        for (uint32_t i = 0; i < sampleCount; ++i)
        {
            const float xi = (i + 0.5f) / sampleCount;
            const float phi = glm::two_pi<float>() *
                glm::fract(i * 0.61803398875f);
            const float radius = std::sqrt(xi);

            const glm::vec3 direction =
                tangent * (radius * std::cos(phi)) +
                bitangent * (radius * std::sin(phi)) +
                normal * std::sqrt(1.0f - xi);

            average += SampleDefaultEnvironment(direction);
        }

        return average / static_cast<float>(sampleCount);
    }

    Hazel::Ref<Hazel::Texture2D> CreateEnvironmentTexture(
        uint32_t width, uint32_t height, bool diffuse)
    {
        std::vector<uint8_t> pixels(width * height * 4);

        for (uint32_t y = 0; y < height; ++y)
        {
            for (uint32_t x = 0; x < width; ++x)
            {
                const glm::vec3 direction = PanoramaDirection(
                    (x + 0.5f) / width,
                    (y + 0.5f) / height);

                const glm::vec3 color = diffuse
                    ? DiffuseEnvironment(direction)
                    : SampleDefaultEnvironment(direction);

                const uint32_t offset = (y * width + x) * 4;

                // Quantize to the current Texture2D RGBA8 storage format.
                for (uint32_t channel = 0; channel < 3; ++channel)
                {
                    pixels[offset + channel] = static_cast<uint8_t>(
                        glm::clamp(color[channel], 0.0f, 1.0f) *
                        255.0f + 0.5f);
                }

                pixels[offset + 3] = 255;
            }
        }

        auto texture = Hazel::Texture2D::Create(width, height);
        texture->SetData(
            pixels.data(), static_cast<uint32_t>(pixels.size()));
        return texture;
    }

    struct ObjVertexKey
    {
        int Position = 0;
        int TexCoord = 0;
        int Normal = 0;

        bool operator==(const ObjVertexKey& other) const
        {
            return Position == other.Position &&
                   TexCoord == other.TexCoord &&
                   Normal == other.Normal;
        }
    };

    struct ObjVertexKeyHash
    {
        size_t operator()(const ObjVertexKey& key) const
        {
            size_t result =
                std::hash<int>{}(key.Position);

            result ^= std::hash<int>{}(key.TexCoord)
                + 0x9e3779b9u
                + (result << 6)
                + (result >> 2);

            result ^= std::hash<int>{}(key.Normal)
                + 0x9e3779b9u
                + (result << 6)
                + (result >> 2);

            return result;
        }
    };

    int ResolveOBJIndex(
        int index,
        size_t count)
    {
        if (index > 0)
            return index - 1;

        if (index < 0)
            return static_cast<int>(count) + index;

        return -1;
    }

    bool ParseOBJVertexToken(
        const std::string& token,
        ObjVertexKey& key)
    {
        key = {};

        std::stringstream stream(token);
        std::string part;
        std::vector<std::string> parts;

        while (std::getline(stream, part, '/'))
            parts.push_back(part);

        if (parts.empty() || parts[0].empty())
            return false;

        try
        {
            key.Position =
                std::stoi(parts[0]);

            if (parts.size() > 1 &&
                !parts[1].empty())
            {
                key.TexCoord =
                    std::stoi(parts[1]);
            }

            if (parts.size() > 2 &&
                !parts[2].empty())
            {
                key.Normal =
                    std::stoi(parts[2]);
            }
        }
        catch (const std::exception&)
        {
            return false;
        }

        return true;
    }

    bool LoadOBJ(
        const std::filesystem::path& path,
        Hazel::MeshData& meshData)
    {
        std::ifstream file(path);

        if (!file.is_open())
        {
            HZ_CORE_ERROR(
                "Could not open OBJ file: {}",
                path.string());

            return false;
        }

        meshData = {};

        std::vector<glm::vec3> positions;
        std::vector<glm::vec2> texCoords;
        std::vector<glm::vec3> normals;

        std::unordered_map<
            ObjVertexKey,
            uint32_t,
            ObjVertexKeyHash>
            vertexCache;

        std::vector<bool> needsGeneratedNormal;

        std::string line;
        uint32_t lineNumber = 0;

        while (std::getline(file, line))
        {
            ++lineNumber;

            if (line.empty() ||
                line[0] == '#')
            {
                continue;
            }

            std::stringstream stream(line);

            std::string command;
            stream >> command;

            if (command == "v")
            {
                glm::vec3 position{ 0.0f };

                stream >>
                    position.x >>
                    position.y >>
                    position.z;

                positions.push_back(position);
            }
            else if (command == "vt")
            {
                glm::vec2 texCoord{ 0.0f };

                stream >>
                    texCoord.x >>
                    texCoord.y;

                texCoords.push_back(texCoord);
            }
            else if (command == "vn")
            {
                glm::vec3 normal{
                    0.0f,
                    1.0f,
                    0.0f
                };

                stream >>
                    normal.x >>
                    normal.y >>
                    normal.z;

                normals.push_back(normal);
            }
            else if (command == "f")
            {
                std::vector<uint32_t> faceIndices;
                std::string token;

                while (stream >> token)
                {
                    if (token[0] == '#')
                        break;

                    ObjVertexKey rawKey;

                    if (!ParseOBJVertexToken(
                            token,
                            rawKey))
                    {
                        HZ_CORE_ERROR(
                            "Invalid OBJ face at line {}",
                            lineNumber);

                        return false;
                    }

                    // Resolve relative indices before caching: the same -1
                    // token can refer to different vertices later in a file.
                    const ObjVertexKey key{
                        ResolveOBJIndex(rawKey.Position, positions.size()),
                        ResolveOBJIndex(rawKey.TexCoord, texCoords.size()),
                        ResolveOBJIndex(rawKey.Normal, normals.size())
                    };

                    if (rawKey.Position == 0 ||
                        key.Position < 0 ||
                        key.Position >= static_cast<int>(positions.size()) ||
                        (rawKey.TexCoord != 0 &&
                            (key.TexCoord < 0 ||
                             key.TexCoord >= static_cast<int>(texCoords.size()))) ||
                        (rawKey.Normal != 0 &&
                            (key.Normal < 0 ||
                             key.Normal >= static_cast<int>(normals.size()))))
                    {
                        HZ_CORE_ERROR(
                            "OBJ index out of range at line {}",
                            lineNumber);
                        return false;
                    }

                    auto cached =
                        vertexCache.find(key);

                    if (cached != vertexCache.end())
                    {
                        faceIndices.push_back(
                            cached->second);

                        continue;
                    }

                    Hazel::MeshVertex vertex;

                    vertex.Position = positions[key.Position];
                    vertex.Normal = glm::vec3(0.0f);

                    if (key.TexCoord >= 0)
                    {
                        // Texture loading already flips image rows.
                        // Preserve the OBJ UVs to avoid a second V flip.
                        vertex.TexCoord = texCoords[key.TexCoord];
                    }

                    if (key.Normal >= 0)
                        vertex.Normal = normals[key.Normal];

                    const uint32_t vertexIndex =
                        static_cast<uint32_t>(
                            meshData.Vertices.size());

                    meshData.Vertices.push_back(
                        vertex);
                    needsGeneratedNormal.push_back(key.Normal < 0);

                    vertexCache.emplace(
                        key,
                        vertexIndex);

                    faceIndices.push_back(
                        vertexIndex);
                }

                if (faceIndices.size() < 3)
                {
                    HZ_CORE_ERROR(
                        "OBJ face has fewer than three vertices at line {}",
                        lineNumber);
                    return false;
                }

                // Fan triangulation supports convex polygon faces.
                for (size_t index = 1;
                     index + 1 < faceIndices.size();
                     ++index)
                {
                    meshData.Indices.push_back(
                        faceIndices[0]);

                    meshData.Indices.push_back(
                        faceIndices[index]);

                    meshData.Indices.push_back(
                        faceIndices[index + 1]);
                }
            }
        }

        if (meshData.Vertices.empty() ||
            meshData.Indices.empty())
        {
            HZ_CORE_ERROR(
                "OBJ file contains no drawable geometry: {}",
                path.string());

            return false;
        }

        // Accumulate area-weighted normals only for vertices without normals.
        if (std::find(needsGeneratedNormal.begin(),
                      needsGeneratedNormal.end(), true) !=
            needsGeneratedNormal.end())
        {
            for (size_t index = 0;
                 index + 2 < meshData.Indices.size();
                 index += 3)
            {
                Hazel::MeshVertex& a =
                    meshData.Vertices[
                        meshData.Indices[index + 0]];

                Hazel::MeshVertex& b =
                    meshData.Vertices[
                        meshData.Indices[index + 1]];

                Hazel::MeshVertex& c =
                    meshData.Vertices[
                        meshData.Indices[index + 2]];

                const glm::vec3 normal =
                    glm::cross(
                        b.Position - a.Position,
                        c.Position - a.Position);

                if (needsGeneratedNormal[meshData.Indices[index + 0]])
                    a.Normal += normal;
                if (needsGeneratedNormal[meshData.Indices[index + 1]])
                    b.Normal += normal;
                if (needsGeneratedNormal[meshData.Indices[index + 2]])
                    c.Normal += normal;
            }

            for (Hazel::MeshVertex& vertex :
                 meshData.Vertices)
            {
                if (glm::length(vertex.Normal) >
                    0.0001f)
                {
                    vertex.Normal =
                        glm::normalize(
                            vertex.Normal);
                }
                else
                {
                    vertex.Normal = {
                        0.0f,
                        1.0f,
                        0.0f
                    };
                }
            }
        }

        meshData.Bounds =
            Hazel::CalculateBounds(
                meshData.Vertices);

        return true;
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
    auto& camera = m_CameraController.GetCamera();

    camera.SetPosition({ 0.0f, 0.0f, 5.0f });
    camera.SetRotation({ 0.0f, 0.0f, 0.0f });

    const auto& window =
        Hazel::Application::Get().GetWindow();

    if (window.GetWidth() > 0 &&
        window.GetHeight() > 0)
    {
        camera.SetViewportSize(
            window.GetWidth(),
            window.GetHeight());
    }

    // 当前先沿用你已有的模型路径。
    // Shader 与模型共用同一个 assets 根目录。
    const std::filesystem::path assetsRoot =
        "D:/Project/cpp/Hazel/Sandbox/assets";

    const auto modelPath =
        assetsRoot /
        "model/Intergalactic_Spaceship-(Wavefront).obj";

    const auto shaderPath =
        assetsRoot /
        "shaders/Sandbox3D_BlinnPhong.glsl";

    // 1. 加载几何。
    Hazel::MeshData modelData;

    if (!LoadOBJ(modelPath, modelData))
        return;

    m_Model = Hazel::Mesh::Create(modelData);

    // 2. 将模型居中，并将最长边缩放到 3 个单位。
    const auto& bounds = m_Model->GetBounds();

    const glm::vec3 center =
        (bounds.Min + bounds.Max) * 0.5f;

    const glm::vec3 size =
        bounds.Max - bounds.Min;

    const float largestExtent =
        std::max(size.x, std::max(size.y, size.z));

    const float modelScale =
        largestExtent > 0.0f
            ? 3.0f / largestExtent
            : 1.0f;

    m_ModelTransform =
        glm::scale(
            glm::mat4(1.0f),
            glm::vec3(modelScale)) *
        glm::translate(
            glm::mat4(1.0f),
            -center);

    // 模型变换当前固定，法线矩阵只计算一次。
    m_NormalMatrix = glm::inverseTranspose(
        glm::mat3(m_ModelTransform));

    // 3. 加载 Shader。
    m_ModelShader =
        Hazel::Shader::Create(shaderPath.string());

    m_EnvironmentShader = Hazel::Shader::Create(
        (assetsRoot / "shaders/Sandbox3D_Environment.glsl").string());
    CreateEnvironment();

    // 4. 创建共享材质和实例。
    m_MaterialSystem =
        std::make_unique<Hazel::MaterialSystem>();

    auto baseMaterial =
        m_MaterialSystem->CreateBlinnPhong(
            m_ModelShader);

    m_ModelMaterial =
        Hazel::CreateRef<Hazel::MaterialInstance>(
            baseMaterial);

    m_ModelMaterial->Set(
        "u_Material.BaseColor",
        glm::vec4(0.65f, 0.72f, 0.86f, 1.0f));

    m_ModelMaterial->Set(
        "u_Material.SpecularStrength",
        0.35f);

    m_ModelMaterial->Set(
        "u_Material.Shininess",
        64.0f);

    // 没有设置贴图覆盖时，
    // MaterialSystem 中的白色纹理会作为默认贴图。

    // 5. 配置灯光。
    m_Lighting = {};

    m_Lighting.Ambient.Color =
        glm::vec3(1.0f);

    // A small constant fill complements the directional environment light.
    m_Lighting.Ambient.Intensity = 0.04f;

    Hazel::DirectionalLight sun;

    sun.Direction = { -0.4f, -0.8f, -0.5f };
    sun.Properties.Color = { 1.0f, 0.95f, 0.9f };
    sun.Properties.Intensity = 0.8f;

    m_Lighting.DirectionalLights.push_back(sun);

    Hazel::PointLight fillLight;

    fillLight.Position = { -2.0f, 1.0f, 2.0f };
    fillLight.Range = 8.0f;

    fillLight.Properties.Color =
        { 0.35f, 0.55f, 1.0f };

    fillLight.Properties.Intensity = 4.0f;

    m_Lighting.PointLights.push_back(fillLight);

    // 聚光灯先准备好，但默认关闭，
    // 可以先观察方向光和点光源的效果。
    Hazel::SpotLight spotLight;

    spotLight.Position = { 2.0f, 2.0f, 3.0f };

    // 模型已经居中，灯光朝向世界原点。
    spotLight.Direction = -spotLight.Position;

    spotLight.Range = 10.0f;

    // 当前 LightingUploader 接收角度制。
    spotLight.InnerConeAngle = 15.0f;
    spotLight.OuterConeAngle = 25.0f;

    spotLight.Properties.Color =
        { 1.0f, 0.65f, 0.3f };

    spotLight.Properties.Intensity = 10.0f;
    spotLight.Properties.Enabled = false;

    m_Lighting.SpotLights.push_back(spotLight);

    HZ_CORE_INFO(
        "Sandbox3D: loaded {} vertices, {} indices",
        m_Model->GetVertexCount(),
        m_Model->GetIndexCount());
}

void Sandbox3D::OnUpdate(Hazel::Timestep timestep)
{
    const auto& io = ImGui::GetIO();
    m_CameraController.SetEnabled(
        !io.WantCaptureMouse && !io.WantCaptureKeyboard);
    m_CameraController.OnUpdate(timestep);

    const auto& window =
        Hazel::Application::Get().GetWindow();

    const uint32_t width = window.GetWidth();
    const uint32_t height = window.GetHeight();

    if (width == 0 || height == 0)
        return;

    Hazel::RenderCommand::SetViewport(
        0, 0, width, height);

    Hazel::RenderCommand::SetClearColor({
        0.035f,
        0.045f,
        0.065f,
        1.0f
    });

    Hazel::RenderCommand::Clear();

    if (m_ShowEnvironment && m_EnvironmentMap)
        RenderEnvironment();

    if (!m_Model || !m_ModelMaterial)
        return;

    RenderModel();
}

void Sandbox3D::CreateEnvironment()
{
    m_EnvironmentMap = CreateEnvironmentTexture(512, 256, false);
    m_IrradianceMap = CreateEnvironmentTexture(32, 16, true);

    float vertices[] = {
        -1.0f, -1.0f,
         1.0f, -1.0f,
         1.0f,  1.0f,
        -1.0f,  1.0f
    };

    uint32_t indices[] = { 0, 1, 2, 2, 3, 0 };

    auto vertexBuffer = Hazel::VertexBuffer::Create(
        vertices, static_cast<uint32_t>(sizeof(vertices)));
    vertexBuffer->SetLayout({
        { Hazel::ShaderDataType::Float2, "a_Position" }
    });

    m_EnvironmentQuad = Hazel::VertexArray::Create();
    m_EnvironmentQuad->AddVertexBuffer(vertexBuffer);
    m_EnvironmentQuad->SetIndexBuffer(
        Hazel::IndexBuffer::Create(indices, 6));

    HZ_CORE_INFO("Sandbox3D: generated environment 512x256, diffuse map 32x16");
}

void Sandbox3D::RenderEnvironment()
{
    auto& camera = m_CameraController.GetCamera();

    m_EnvironmentShader->Bind();
    m_EnvironmentShader->SetMat4(
        "u_InverseProjection",
        glm::inverse(camera.GetProjectionMatrix()));
    m_EnvironmentShader->SetMat3(
        "u_InverseViewRotation",
        glm::transpose(glm::mat3(camera.GetViewMatrix())));
    m_EnvironmentShader->SetFloat(
        "u_EnvironmentRotation", glm::radians(m_EnvironmentRotation));
    m_EnvironmentShader->SetFloat("u_Intensity", m_BackgroundIntensity);

    m_EnvironmentMap->Bind(0);
    m_EnvironmentShader->SetInt("u_EnvironmentMap", 0);

    // The background must not write scene depth or inherit alpha blending.
    // Keep these temporary backend calls local to this Sandbox experiment.
    const GLboolean depthTest = glIsEnabled(GL_DEPTH_TEST);
    const GLboolean blend = glIsEnabled(GL_BLEND);
    GLboolean depthWrite = GL_TRUE;
    glGetBooleanv(GL_DEPTH_WRITEMASK, &depthWrite);

    glDisable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
    glDepthMask(GL_FALSE);

    m_EnvironmentQuad->Bind();
    Hazel::RenderCommand::DrawIndexed(m_EnvironmentQuad, 6);

    glDepthMask(depthWrite);
    if (depthTest)
        glEnable(GL_DEPTH_TEST);
    if (blend)
        glEnable(GL_BLEND);
}

void Sandbox3D::RenderModel()
{
    auto& camera =
        m_CameraController.GetCamera();

    // 1. 绑定材质 Shader，上传材质数值和纹理。
    const uint32_t nextTextureSlot =
        Hazel::MaterialBinder::Bind(
            *m_ModelMaterial,
            0,
            14); // Reserve two of the 16 slots for environment maps.

    // Binder 已经绑定了此 Shader。
    auto& shader =
        *m_ModelMaterial->GetMaterial()->GetShader();

    // 2. 上传相机参数。
    shader.SetMat4(
        "u_ViewProjection",
        camera.GetViewProjectionMatrix());

    // 适配当前 SetFloat3 的非 const 引用接口。
    glm::vec3 cameraPosition =
        camera.GetPosition();

    shader.SetFloat3(
        "u_CameraPosition",
        cameraPosition);

    // 3. 上传灯光。
    const auto lightingStats =
        Hazel::LightingUploader::Upload(
            shader,
            m_Lighting);

    (void)lightingStats;

    m_EnvironmentMap->Bind(nextTextureSlot);
    m_IrradianceMap->Bind(nextTextureSlot + 1);

    shader.SetInt("u_EnvironmentMap", static_cast<int>(nextTextureSlot));
    shader.SetInt("u_IrradianceMap", static_cast<int>(nextTextureSlot + 1));
    shader.SetInt("u_EnvironmentEnabled", m_EnvironmentLighting ? 1 : 0);
    shader.SetFloat("u_EnvironmentIntensity", m_EnvironmentIntensity);
    shader.SetFloat("u_ReflectionStrength", m_ReflectionStrength);
    shader.SetFloat(
        "u_EnvironmentRotation", glm::radians(m_EnvironmentRotation));

    // 4. 上传物体变换。
    shader.SetMat4(
        "u_Model",
        m_ModelTransform);

    shader.SetMat3(
        "u_NormalMatrix",
        m_NormalMatrix);

    // 5. 提交几何。
    const auto& vertexArray =
        m_Model->GetVertexArray();

    vertexArray->Bind();

    Hazel::RenderCommand::DrawIndexed(
        vertexArray,
        m_Model->GetIndexCount());
}

void Sandbox3D::OnImGuiRender()
{
    ImGui::Begin("Environment", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
    ImGui::PushItemWidth(240.0f);

    ImGui::ColorEdit3(
        "Ambient color", glm::value_ptr(m_Lighting.Ambient.Color));
    ImGui::SliderFloat("Ambient intensity", &m_Lighting.Ambient.Intensity,
        0.0f, 1.0f, "%.2f", ImGuiSliderFlags_AlwaysClamp);

    ImGui::Separator();
    ImGui::Checkbox("Show background", &m_ShowEnvironment);
    ImGui::Checkbox("Environment lighting", &m_EnvironmentLighting);
    ImGui::SliderFloat("Diffuse intensity", &m_EnvironmentIntensity,
        0.0f, 2.0f, "%.2f", ImGuiSliderFlags_AlwaysClamp);
    ImGui::SliderFloat("Reflection strength", &m_ReflectionStrength,
        0.0f, 1.0f, "%.2f", ImGuiSliderFlags_AlwaysClamp);
    ImGui::SliderFloat("Background intensity", &m_BackgroundIntensity,
        0.0f, 2.0f, "%.2f", ImGuiSliderFlags_AlwaysClamp);
    ImGui::SliderFloat("Environment rotation", &m_EnvironmentRotation,
        -180.0f, 180.0f, "%.0f deg", ImGuiSliderFlags_AlwaysClamp);

    ImGui::PopItemWidth();
    ImGui::End();
}

void Sandbox3D::OnEvent(Hazel::Event& event)
{
    // The current camera controller divides by height on resize.
    // Ignore minimized dimensions until a non-zero resize arrives.
    if (event.GetEventType() == Hazel::EventType::WindowResize)
    {
        const auto& resize =
            static_cast<const Hazel::WindowResizeEvent&>(event);

        if (resize.GetWidth() == 0 || resize.GetHeight() == 0)
            return;
    }

    const auto& io = ImGui::GetIO();
    if ((io.WantCaptureMouse &&
         event.IsInCategory(Hazel::EventCategoryMouse)) ||
        (io.WantCaptureKeyboard &&
         event.IsInCategory(Hazel::EventCategoryKeyboard)))
    {
        return;
    }

    m_CameraController.OnEvent(event);
}

void Sandbox3D::OnDetach()
{
    m_EnvironmentQuad.reset();
    m_EnvironmentShader.reset();
    m_IrradianceMap.reset();
    m_EnvironmentMap.reset();

    m_ModelMaterial.reset();
    m_MaterialSystem.reset();

    m_ModelShader.reset();
    m_Model.reset();
}
