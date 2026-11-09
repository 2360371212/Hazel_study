#include "Sandbox3D.h"

#include "Hazel/Events/ApplicationEvent.h"
#include "Hazel/Renderer/Renderer3D.h"
#include "Hazel/Renderer/RenderView.h"
#include "Hazel/Renderer/Scene/SceneRenderExtractor.h"

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
#include <utility>
#include <vector>

#include <glm/gtc/constants.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtx/quaternion.hpp>

#include <glad/glad.h>
#include <imgui.h>

namespace
{
    // ============================================================
    // 程序化环境
    // ============================================================

    glm::vec3 PanoramaDirection(float u, float v)
    {
        const float azimuth =
            (u - 0.5f) * glm::two_pi<float>();

        const float elevation =
            (v - 0.5f) * glm::pi<float>();

        const float radius = std::cos(elevation);

        return {
            radius * std::cos(azimuth),
            std::sin(elevation),
            radius * std::sin(azimuth)
        };
    }

    glm::vec3 SampleDefaultEnvironment(
        const glm::vec3& direction)
    {
        const glm::vec3 horizon{
            0.65f, 0.72f, 0.8f
        };

        const glm::vec3 zenith{
            0.08f, 0.24f, 0.55f
        };

        const glm::vec3 ground{
            0.12f, 0.10f, 0.08f
        };

        glm::vec3 color =
            direction.y >= 0.0f
                ? glm::mix(
                    horizon,
                    zenith,
                    std::sqrt(direction.y))
                : glm::mix(
                    horizon,
                    ground,
                    std::sqrt(-direction.y));

        const glm::vec3 sunDirection =
            glm::normalize(
                glm::vec3(0.4f, 0.8f, 0.5f));

        const float sun = std::pow(
            std::max(
                glm::dot(direction, sunDirection),
                0.0f),
            512.0f);

        return color +
            glm::vec3(0.65f, 0.5f, 0.3f) * sun;
    }

    glm::vec3 DiffuseEnvironment(
        const glm::vec3& normal)
    {
        const glm::vec3 up =
            std::abs(normal.y) < 0.999f
                ? glm::vec3(0.0f, 1.0f, 0.0f)
                : glm::vec3(1.0f, 0.0f, 0.0f);

        const glm::vec3 tangent =
            glm::normalize(
                glm::cross(up, normal));

        const glm::vec3 bitangent =
            glm::cross(normal, tangent);

        constexpr uint32_t sampleCount = 256;

        glm::vec3 average{ 0.0f };

        // 余弦加权半球采样，结果为 irradiance / pi。
        // 仅在创建环境时计算。
        for (uint32_t i = 0; i < sampleCount; ++i)
        {
            const float xi =
                (i + 0.5f) / sampleCount;

            const float phi =
                glm::two_pi<float>() *
                glm::fract(i * 0.61803398875f);

            const float radius = std::sqrt(xi);

            const glm::vec3 direction =
                tangent * (radius * std::cos(phi)) +
                bitangent * (radius * std::sin(phi)) +
                normal * std::sqrt(1.0f - xi);

            average +=
                SampleDefaultEnvironment(direction);
        }

        return average /
            static_cast<float>(sampleCount);
    }

    Hazel::Ref<Hazel::Texture2D>
    CreateEnvironmentTexture(
        uint32_t width,
        uint32_t height,
        bool diffuse)
    {
        std::vector<uint8_t> pixels(
            width * height * 4);

        for (uint32_t y = 0; y < height; ++y)
        {
            for (uint32_t x = 0; x < width; ++x)
            {
                const glm::vec3 direction =
                    PanoramaDirection(
                        (x + 0.5f) / width,
                        (y + 0.5f) / height);

                const glm::vec3 color =
                    diffuse
                        ? DiffuseEnvironment(direction)
                        : SampleDefaultEnvironment(direction);

                const uint32_t offset =
                    (y * width + x) * 4;

                // 当前 Texture2D 使用 RGBA8。
                // 此处保存的是线性颜色，首行为全景图底部。
                for (uint32_t channel = 0;
                     channel < 3;
                     ++channel)
                {
                    pixels[offset + channel] =
                        static_cast<uint8_t>(
                            glm::clamp(
                                color[channel],
                                0.0f,
                                1.0f) *
                            255.0f + 0.5f);
                }

                pixels[offset + 3] = 255;
            }
        }

        auto texture =
            Hazel::Texture2D::Create(
                width,
                height);

        texture->SetData(
            pixels.data(),
            static_cast<uint32_t>(pixels.size()));

        return texture;
    }

    // ============================================================
    // OBJ 测试加载器
    // ============================================================

    struct ObjVertexKey
    {
        int Position = 0;
        int TexCoord = 0;
        int Normal = 0;

        bool operator==(
            const ObjVertexKey& other) const
        {
            return
                Position == other.Position &&
                TexCoord == other.TexCoord &&
                Normal == other.Normal;
        }
    };

    struct ObjVertexKeyHash
    {
        size_t operator()(
            const ObjVertexKey& key) const
        {
            size_t result =
                std::hash<int>{}(key.Position);

            result ^=
                std::hash<int>{}(key.TexCoord) +
                0x9e3779b9u +
                (result << 6) +
                (result >> 2);

            result ^=
                std::hash<int>{}(key.Normal) +
                0x9e3779b9u +
                (result << 6) +
                (result >> 2);

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
            key.Position = std::stoi(parts[0]);

            if (parts.size() > 1 &&
                !parts[1].empty())
            {
                key.TexCoord = std::stoi(parts[1]);
            }

            if (parts.size() > 2 &&
                !parts[2].empty())
            {
                key.Normal = std::stoi(parts[2]);
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
            ObjVertexKeyHash> vertexCache;

        std::vector<bool> needsGeneratedNormal;

        std::string line;
        uint32_t lineNumber = 0;

        while (std::getline(file, line))
        {
            ++lineNumber;

            std::stringstream stream(line);

            std::string command;
            stream >> command;

            if (command.empty() || command[0] == '#')
                continue;

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
                    0.0f, 1.0f, 0.0f
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

                    // 必须先解析负索引，再用于顶点缓存。
                    const ObjVertexKey key{
                        ResolveOBJIndex(
                            rawKey.Position,
                            positions.size()),

                        ResolveOBJIndex(
                            rawKey.TexCoord,
                            texCoords.size()),

                        ResolveOBJIndex(
                            rawKey.Normal,
                            normals.size())
                    };

                    if (
                        rawKey.Position == 0 ||
                        key.Position < 0 ||
                        key.Position >=
                            static_cast<int>(positions.size()) ||

                        (rawKey.TexCoord != 0 &&
                            (key.TexCoord < 0 ||
                             key.TexCoord >=
                                static_cast<int>(texCoords.size()))) ||

                        (rawKey.Normal != 0 &&
                            (key.Normal < 0 ||
                             key.Normal >=
                                static_cast<int>(normals.size())))
                    )
                    {
                        HZ_CORE_ERROR(
                            "OBJ index out of range at line {}",
                            lineNumber);

                        return false;
                    }

                    const auto cached =
                        vertexCache.find(key);

                    if (cached != vertexCache.end())
                    {
                        faceIndices.push_back(
                            cached->second);

                        continue;
                    }

                    Hazel::MeshVertex vertex;

                    vertex.Position =
                        positions[key.Position];

                    vertex.Normal =
                        glm::vec3(0.0f);

                    if (key.TexCoord >= 0)
                    {
                        // 纹理加载已翻转图像行，不再翻转 OBJ UV。
                        vertex.TexCoord =
                            texCoords[key.TexCoord];
                    }

                    if (key.Normal >= 0)
                    {
                        vertex.Normal =
                            normals[key.Normal];
                    }

                    const uint32_t vertexIndex =
                        static_cast<uint32_t>(
                            meshData.Vertices.size());

                    meshData.Vertices.push_back(vertex);

                    needsGeneratedNormal.push_back(
                        key.Normal < 0);

                    vertexCache.emplace(
                        key,
                        vertexIndex);

                    faceIndices.push_back(vertexIndex);
                }

                if (faceIndices.size() < 3)
                {
                    HZ_CORE_ERROR(
                        "OBJ face has fewer than three vertices at line {}",
                        lineNumber);

                    return false;
                }

                // 与原测试一致：凸多边形使用扇形三角化。
                for (size_t i = 1;
                     i + 1 < faceIndices.size();
                     ++i)
                {
                    meshData.Indices.insert(
                        meshData.Indices.end(),
                        {
                            faceIndices[0],
                            faceIndices[i],
                            faceIndices[i + 1]
                        });
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

        // 只为没有法线的顶点累积面积加权法线。
        if (std::find(
                needsGeneratedNormal.begin(),
                needsGeneratedNormal.end(),
                true) != needsGeneratedNormal.end())
        {
            for (size_t i = 0;
                 i + 2 < meshData.Indices.size();
                 i += 3)
            {
                const uint32_t ia =
                    meshData.Indices[i];

                const uint32_t ib =
                    meshData.Indices[i + 1];

                const uint32_t ic =
                    meshData.Indices[i + 2];

                auto& a = meshData.Vertices[ia];
                auto& b = meshData.Vertices[ib];
                auto& c = meshData.Vertices[ic];

                const glm::vec3 normal =
                    glm::cross(
                        b.Position - a.Position,
                        c.Position - a.Position);

                if (needsGeneratedNormal[ia])
                    a.Normal += normal;

                if (needsGeneratedNormal[ib])
                    b.Normal += normal;

                if (needsGeneratedNormal[ic])
                    c.Normal += normal;
            }

            for (auto& vertex : meshData.Vertices)
            {
                const float lengthSquared =
                    glm::dot(
                        vertex.Normal,
                        vertex.Normal);

                vertex.Normal =
                    lengthSquared > 1e-8f
                        ? vertex.Normal / std::sqrt(lengthSquared)
                        : glm::vec3(0.0f, 1.0f, 0.0f);
            }
        }

        meshData.Bounds =
            Hazel::CalculateBounds(
                meshData.Vertices);

        return true;
    }
}

// ============================================================
// 初始化
// ============================================================

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

    const auto& window =
        Hazel::Application::Get().GetWindow();

    if (window.GetWidth() > 0 &&
        window.GetHeight() > 0)
    {
        camera.SetViewportSize(
            window.GetWidth(),
            window.GetHeight());
    }

    // 当前 CMake 使用绝对源文件路径。
    // 从 Sandbox/src/Sandbox3D.cpp 向上定位项目根目录。
    // 这是开发期源码资源路径，不是发布包资源路径。
    const std::filesystem::path projectRoot =
        std::filesystem::path(__FILE__)
            .parent_path()
            .parent_path()
            .parent_path();

    const auto engineAssetsRoot =
        projectRoot / "Hazel/src/Hazel/Assets";

    const auto sandboxAssetsRoot =
        projectRoot / "Sandbox/assets";

    m_Scene = std::make_unique<Hazel::Scene>();

    m_MaterialSystem =
        std::make_unique<Hazel::MaterialSystem>();

    // PBR 测试先初始化，不依赖 OBJ 是否加载成功。
    CreatePBRTest(engineAssetsRoot);

    CreateEnvironment(sandboxAssetsRoot);
    CreateModelTest(sandboxAssetsRoot);

    BuildRenderScene();
    SyncRenderScene();

    ResetTestCamera();
}

void Sandbox3D::CreatePBRTest(
    const std::filesystem::path& engineAssetsRoot)
{
    m_PBRShader = Hazel::Shader::Create(
        (
            engineAssetsRoot /
            "Shaders/PBR.glsl"
        ).string());

    // 所有球体共享同一份 Mesh。
    m_PBRSphere =
        Hazel::PrimitiveMesh::CreateSphere(
            0.85f,
            64,
            32);

    // 所有实例共享同一个基础材质。
    const auto baseMaterial =
        m_MaterialSystem->CreatePBR(
            m_PBRShader);

    constexpr uint32_t rows = 7;
    constexpr uint32_t columns = 7;

    constexpr float spacing = 2.3f;

    const float halfRows =
        static_cast<float>(rows - 1) * 0.5f;

    const float halfColumns =
        static_cast<float>(columns - 1) * 0.5f;

    m_PBRObjects.clear();
    m_PBRObjects.reserve(rows * columns);

    for (uint32_t row = 0; row < rows; ++row)
    {
        const float metallic =
            static_cast<float>(row) /
            static_cast<float>(rows - 1);

        for (uint32_t column = 0;
             column < columns;
             ++column)
        {
            const float roughness = glm::mix(
                0.05f,
                1.0f,
                static_cast<float>(column) /
                static_cast<float>(columns - 1));

            PBRTestObject object;

            object.Material =
                Hazel::CreateRef<Hazel::MaterialInstance>(
                    baseMaterial);

            object.Material->Set(
                "u_Material.BaseColor",
                glm::vec4(m_PBRBaseColor, 1.0f));

            object.Material->Set(
                "u_Material.Metallic",
                metallic);

            object.Material->Set(
                "u_Material.Roughness",
                roughness);

            const glm::vec3 position{
                (
                    static_cast<float>(column) -
                    halfColumns
                ) * spacing,

                (
                    static_cast<float>(row) -
                    halfRows
                ) * spacing,

                0.0f
            };

            object.Transform = glm::translate(
                glm::mat4(1.0f),
                position);

            object.NormalMatrix =
                glm::inverseTranspose(
                    glm::mat3(object.Transform));

            m_PBRObjects.push_back(
                std::move(object));
        }
    }

    // 独立的 PBR 测试灯光。
    m_PBRLighting = {};

    m_PBRLighting.Ambient.Intensity = 0.0f;

    Hazel::DirectionalLight sun;

    sun.Direction = {
        -0.3f, -0.5f, -1.0f
    };

    sun.Properties.Intensity = 2.0f;

    m_PBRLighting.DirectionalLights.push_back(sun);

    const glm::vec3 pointPositions[] = {
        { -8.0f,  8.0f, 10.0f },
        {  8.0f,  8.0f, 10.0f },
        { -8.0f, -8.0f, 10.0f },
        {  8.0f, -8.0f, 10.0f }
    };

    for (const auto& position : pointPositions)
    {
        Hazel::PointLight light;

        light.Position = position;
        light.Range = 40.0f;

        light.Properties.Intensity = 100.0f;

        m_PBRLighting.PointLights.push_back(light);
    }

    Hazel::SpotLight spot;

    spot.Position = {
        0.0f, 8.0f, 10.0f
    };

    spot.Direction = -spot.Position;

    spot.Range = 40.0f;

    spot.InnerConeAngle = 15.0f;
    spot.OuterConeAngle = 25.0f;

    spot.Properties.Color = {
        1.0f, 0.65f, 0.3f
    };

    spot.Properties.Intensity = 120.0f;
    spot.Properties.Enabled = false;

    m_PBRLighting.SpotLights.push_back(spot);
}

void Sandbox3D::CreateModelTest(
    const std::filesystem::path& sandboxAssetsRoot)
{
    Hazel::MeshData data;

    const auto modelPath =
        sandboxAssetsRoot /
        "model/Intergalactic_Spaceship-(Wavefront).obj";

    if (!LoadOBJ(modelPath, data))
        return;

    m_Model =
        Hazel::Mesh::Create(data);

    const auto& bounds =
        m_Model->GetBounds();

    const glm::vec3 center =
        (bounds.Min + bounds.Max) * 0.5f;

    const glm::vec3 size =
        bounds.Max - bounds.Min;

    const float extent =
        std::max(
            size.x,
            std::max(size.y, size.z));

    const float scale =
        extent > 0.0f
            ? 3.0f / extent
            : 1.0f;

    m_ModelTransform =
        glm::scale(
            glm::mat4(1.0f),
            glm::vec3(scale)) *
        glm::translate(
            glm::mat4(1.0f),
            -center);

    m_NormalMatrix =
        glm::inverseTranspose(
            glm::mat3(m_ModelTransform));

    m_ModelShader = Hazel::Shader::Create(
        (
            sandboxAssetsRoot /
            "shaders/Sandbox3D_BlinnPhong.glsl"
        ).string());

    const auto material =
        m_MaterialSystem->CreateBlinnPhong(
            m_ModelShader);

    m_ModelMaterial =
        Hazel::CreateRef<Hazel::MaterialInstance>(
            material);

    m_ModelMaterial->Set(
        "u_Material.BaseColor",
        glm::vec4(0.65f, 0.72f, 0.86f, 1.0f));

    m_ModelMaterial->Set(
        "u_Material.SpecularStrength",
        0.35f);

    m_ModelMaterial->Set(
        "u_Material.Shininess",
        64.0f);

    m_Lighting = {};

    m_Lighting.Ambient.Intensity = 0.04f;

    Hazel::DirectionalLight sun;

    sun.Direction = {
        -0.4f, -0.8f, -0.5f
    };

    sun.Properties.Color = {
        1.0f, 0.95f, 0.9f
    };

    sun.Properties.Intensity = 0.8f;

    m_Lighting.DirectionalLights.push_back(sun);

    Hazel::PointLight fill;

    fill.Position = {
        -2.0f, 1.0f, 2.0f
    };

    fill.Range = 8.0f;

    fill.Properties.Color = {
        0.35f, 0.55f, 1.0f
    };

    fill.Properties.Intensity = 4.0f;

    m_Lighting.PointLights.push_back(fill);

    Hazel::SpotLight spot;

    spot.Position = {
        2.0f, 2.0f, 3.0f
    };

    spot.Direction = -spot.Position;

    spot.Range = 10.0f;

    spot.InnerConeAngle = 15.0f;
    spot.OuterConeAngle = 25.0f;

    spot.Properties.Color = {
        1.0f, 0.65f, 0.3f
    };

    spot.Properties.Intensity = 10.0f;
    spot.Properties.Enabled = false;

    m_Lighting.SpotLights.push_back(spot);
}

void Sandbox3D::BuildRenderScene()
{
    m_Scene->Clear();

    auto applyMatrix = [](
        const glm::mat4& matrix,
        Hazel::TransformComponent& transform)
    {
        glm::vec3 scale;
        glm::vec3 translation;
        glm::vec3 skew;
        glm::vec4 perspective;
        glm::quat rotation;

        if (glm::decompose(
                matrix,
                scale,
                rotation,
                translation,
                skew,
                perspective))
        {
            transform.Position = translation;
            transform.Scale = scale;
            transform.Rotation = glm::eulerAngles(rotation);
        }
    };

    auto directionToRotation = [](
        glm::vec3 direction)
    {
        if (glm::dot(direction, direction) < 0.000001f)
            return glm::vec3(0.0f);

        direction = glm::normalize(direction);

        const glm::vec3 up =
            std::abs(direction.y) < 0.99f
                ? glm::vec3(0.0f, 1.0f, 0.0f)
                : glm::vec3(1.0f, 0.0f, 0.0f);

        return glm::eulerAngles(
            glm::quatLookAt(direction, up));
    };

    m_PBRSceneEntities.clear();
    m_PBRScenePointLights.clear();

    for (const auto& object : m_PBRObjects)
    {
        auto entity =
            m_Scene->CreateEntity(
                "PBR Sphere");

        auto& transform =
            entity.AddComponent<
                Hazel::TransformComponent>();

        applyMatrix(
            object.Transform,
            transform);

        auto& renderer =
            entity.AddComponent<
                Hazel::MeshRendererComponent>();

        renderer.MeshResource =
            m_PBRSphere;

        renderer.Material =
            object.Material;

        m_PBRSceneEntities.push_back(
            entity);
    }

    m_PBRSceneDirectionalLight =
        m_Scene->CreateEntity(
            "PBR Directional Light");

    {
        auto& source =
            m_PBRLighting.DirectionalLights.front();

        auto& transform =
            m_PBRSceneDirectionalLight.AddComponent<
                Hazel::TransformComponent>();

        transform.Rotation =
            directionToRotation(source.Direction);

        auto& light =
            m_PBRSceneDirectionalLight.AddComponent<
                Hazel::DirectionalLightComponent>();

        light.Properties =
            source.Properties;
    }

    for (const auto& source : m_PBRLighting.PointLights)
    {
        auto entity =
            m_Scene->CreateEntity(
                "PBR Point Light");

        auto& transform =
            entity.AddComponent<
                Hazel::TransformComponent>();

        transform.Position =
            source.Position;

        auto& light =
            entity.AddComponent<
                Hazel::PointLightComponent>();

        light.Properties =
            source.Properties;

        light.Range =
            source.Range;

        m_PBRScenePointLights.push_back(
            entity);
    }

    m_PBRSceneSpotLight =
        m_Scene->CreateEntity(
            "PBR Spot Light");

    {
        auto& source =
            m_PBRLighting.SpotLights.front();

        auto& transform =
            m_PBRSceneSpotLight.AddComponent<
                Hazel::TransformComponent>();

        transform.Position =
            source.Position;

        transform.Rotation =
            directionToRotation(source.Direction);

        auto& light =
            m_PBRSceneSpotLight.AddComponent<
                Hazel::SpotLightComponent>();

        light.Properties =
            source.Properties;

        light.Range =
            source.Range;

        light.InnerConeAngle =
            source.InnerConeAngle;

        light.OuterConeAngle =
            source.OuterConeAngle;
    }

    if (m_Model && m_ModelMaterial)
    {
        m_ModelSceneEntity =
            m_Scene->CreateEntity(
                "Spaceship");

        auto& transform =
            m_ModelSceneEntity.AddComponent<
                Hazel::TransformComponent>();

        applyMatrix(
            m_ModelTransform,
            transform);

        auto& renderer =
            m_ModelSceneEntity.AddComponent<
                Hazel::MeshRendererComponent>();

        renderer.MeshResource =
            m_Model;

        renderer.Material =
            m_ModelMaterial;

        m_ModelSceneDirectionalLight =
            m_Scene->CreateEntity(
                "Model Directional Light");

        auto& directionalSource =
            m_Lighting.DirectionalLights.front();

        auto& directionalTransform =
            m_ModelSceneDirectionalLight.AddComponent<
                Hazel::TransformComponent>();

        directionalTransform.Rotation =
            directionToRotation(
                directionalSource.Direction);

        auto& directionalLight =
            m_ModelSceneDirectionalLight.AddComponent<
                Hazel::DirectionalLightComponent>();

        directionalLight.Properties =
            directionalSource.Properties;

        m_ModelScenePointLight =
            m_Scene->CreateEntity(
                "Model Point Light");

        auto& pointSource =
            m_Lighting.PointLights.front();

        auto& pointTransform =
            m_ModelScenePointLight.AddComponent<
                Hazel::TransformComponent>();

        pointTransform.Position =
            pointSource.Position;

        auto& pointLight =
            m_ModelScenePointLight.AddComponent<
                Hazel::PointLightComponent>();

        pointLight.Properties =
            pointSource.Properties;

        pointLight.Range =
            pointSource.Range;

        m_ModelSceneSpotLight =
            m_Scene->CreateEntity(
                "Model Spot Light");

        auto& spotSource =
            m_Lighting.SpotLights.front();

        auto& spotTransform =
            m_ModelSceneSpotLight.AddComponent<
                Hazel::TransformComponent>();

        spotTransform.Position =
            spotSource.Position;

        spotTransform.Rotation =
            directionToRotation(
                spotSource.Direction);

        auto& spotLight =
            m_ModelSceneSpotLight.AddComponent<
                Hazel::SpotLightComponent>();

        spotLight.Properties =
            spotSource.Properties;

        spotLight.Range =
            spotSource.Range;

        spotLight.InnerConeAngle =
            spotSource.InnerConeAngle;

        spotLight.OuterConeAngle =
            spotSource.OuterConeAngle;
    }
}

void Sandbox3D::SyncRenderScene()
{
    if (!m_Scene)
        return;

    const bool pbrEnabled =
        m_ShowPBRTest;

    for (auto& entity : m_PBRSceneEntities)
        entity.SetEnabled(pbrEnabled);

    if (m_PBRSceneDirectionalLight)
        m_PBRSceneDirectionalLight.SetEnabled(pbrEnabled);

    for (auto& entity : m_PBRScenePointLights)
        entity.SetEnabled(pbrEnabled);

    if (m_PBRSceneSpotLight)
        m_PBRSceneSpotLight.SetEnabled(pbrEnabled);

    if (m_ModelSceneEntity)
        m_ModelSceneEntity.SetEnabled(!pbrEnabled);

    if (m_ModelSceneDirectionalLight)
        m_ModelSceneDirectionalLight.SetEnabled(!pbrEnabled);

    if (m_ModelScenePointLight)
        m_ModelScenePointLight.SetEnabled(!pbrEnabled);

    if (m_ModelSceneSpotLight)
        m_ModelSceneSpotLight.SetEnabled(!pbrEnabled);

    if (m_PBRSceneDirectionalLight &&
        !m_PBRLighting.DirectionalLights.empty())
    {
        auto& component =
            m_PBRSceneDirectionalLight.GetComponent<
                Hazel::DirectionalLightComponent>();

        component.Properties =
            m_PBRLighting.DirectionalLights.front().Properties;
    }

    for (size_t i = 0;
         i < m_PBRScenePointLights.size() &&
             i < m_PBRLighting.PointLights.size();
         ++i)
    {
        auto& component =
            m_PBRScenePointLights[i].GetComponent<
                Hazel::PointLightComponent>();

        component.Properties =
            m_PBRLighting.PointLights[i].Properties;

        component.Range =
            m_PBRLighting.PointLights[i].Range;
    }

    if (m_PBRSceneSpotLight &&
        !m_PBRLighting.SpotLights.empty())
    {
        auto& source =
            m_PBRLighting.SpotLights.front();

        auto& component =
            m_PBRSceneSpotLight.GetComponent<
                Hazel::SpotLightComponent>();

        component.Properties =
            source.Properties;

        component.Range =
            source.Range;

        component.InnerConeAngle =
            source.InnerConeAngle;

        component.OuterConeAngle =
            source.OuterConeAngle;
    }

    if (m_ModelSceneDirectionalLight &&
        !m_Lighting.DirectionalLights.empty())
    {
        auto& component =
            m_ModelSceneDirectionalLight.GetComponent<
                Hazel::DirectionalLightComponent>();

        component.Properties =
            m_Lighting.DirectionalLights.front().Properties;
    }

    if (m_ModelScenePointLight &&
        !m_Lighting.PointLights.empty())
    {
        auto& component =
            m_ModelScenePointLight.GetComponent<
                Hazel::PointLightComponent>();

        component.Properties =
            m_Lighting.PointLights.front().Properties;

        component.Range =
            m_Lighting.PointLights.front().Range;
    }

    if (m_ModelSceneSpotLight &&
        !m_Lighting.SpotLights.empty())
    {
        auto& source =
            m_Lighting.SpotLights.front();

        auto& component =
            m_ModelSceneSpotLight.GetComponent<
                Hazel::SpotLightComponent>();

        component.Properties =
            source.Properties;

        component.Range =
            source.Range;

        component.InnerConeAngle =
            source.InnerConeAngle;

        component.OuterConeAngle =
            source.OuterConeAngle;
    }

    auto& environment =
        m_Scene->GetEnvironment();

    const auto& lighting =
        pbrEnabled ? m_PBRLighting : m_Lighting;

    environment.ClearColor = {
        0.035f,
        0.045f,
        0.065f,
        1.0f
    };

    environment.AmbientColor =
        lighting.Ambient.Color;

    environment.AmbientIntensity =
        lighting.Ambient.Intensity;

    environment.EnvironmentMap =
        m_EnvironmentMap;

    environment.IrradianceMap =
        m_IrradianceMap;

    environment.ShowBackground =
        m_ShowEnvironment;

    environment.EnvironmentLightingEnabled =
        m_EnvironmentLighting;

    environment.EnvironmentIntensity =
        m_EnvironmentIntensity;

    environment.BackgroundIntensity =
        m_BackgroundIntensity;

    environment.RotationRadians =
        glm::radians(m_EnvironmentRotation);

    environment.Exposure =
        m_PBRExposure;
}

void Sandbox3D::CreateEnvironment(
    const std::filesystem::path& sandboxAssetsRoot)
{
    m_EnvironmentShader = Hazel::Shader::Create(
        (
            sandboxAssetsRoot /
            "shaders/Sandbox3D_Environment.glsl"
        ).string());

    m_EnvironmentMap =
        CreateEnvironmentTexture(
            512,
            256,
            false);

    m_IrradianceMap =
        CreateEnvironmentTexture(
            32,
            16,
            true);

    // 使用覆盖整个屏幕的单个三角形，避免全屏四边形对角线接缝。
    float vertices[] = {
        -1.0f, -1.0f,
         3.0f, -1.0f,
        -1.0f,  3.0f
    };

    auto buffer =
        Hazel::VertexBuffer::Create(
            vertices,
            static_cast<uint32_t>(sizeof(vertices)));

    buffer->SetLayout({
        {
            Hazel::ShaderDataType::Float2,
            "a_Position"
        }
    });

    m_EnvironmentQuad =
        Hazel::VertexArray::Create();

    m_EnvironmentQuad->AddVertexBuffer(buffer);
}

void Sandbox3D::ResetTestCamera()
{
    auto& camera =
        m_CameraController.GetCamera();

    camera.SetPosition(
        m_ShowPBRTest
            ? glm::vec3(0.0f, 0.0f, 24.0f)
            : glm::vec3(0.0f, 0.0f, 5.0f));

    camera.SetRotation(
        glm::vec3(0.0f));
}

// ============================================================
// 每帧绘制
// ============================================================

void Sandbox3D::OnUpdate(
    Hazel::Timestep timestep)
{
    const auto& window =
        Hazel::Application::Get().GetWindow();

    const uint32_t width =
        window.GetWidth();

    const uint32_t height =
        window.GetHeight();

    if (width == 0 || height == 0)
    {
        m_CameraController.SetEnabled(false);
        return;
    }

    const auto& io =
        ImGui::GetIO();

    m_CameraController.SetEnabled(
        !io.WantCaptureMouse &&
        !io.WantCaptureKeyboard);

    m_CameraController.OnUpdate(timestep);

    SyncRenderScene();

    Hazel::RenderCommand::SetViewport(
        0, 0, width, height);

    // 当前 PBR Shader 在片元阶段执行显示编码。
    glDisable(GL_FRAMEBUFFER_SRGB);

    Hazel::RenderCommand::SetClearColor(
        m_Scene->GetEnvironment().ClearColor);

    Hazel::RenderCommand::Clear();

    if (m_Scene->GetEnvironment().ShowBackground)
        RenderEnvironment();

    m_TransformSystem.Update(
        *m_Scene);

    Hazel::SceneRenderExtractor::Extract(
        *m_Scene,
        m_TransformSystem,
        m_RenderSceneData);

    const Hazel::RenderView view =
        Hazel::RenderView::FromCamera(
            m_CameraController.GetCamera(),
            width,
            height);

    Hazel::Renderer3DSettings settings;
    settings.ClearTarget = false;
    settings.Exposure = m_PBRExposure;
    settings.ReflectionStrength =
        m_ReflectionStrength;

    Hazel::Renderer3D::Render(
        m_RenderSceneData,
        view,
        settings);
}

void Sandbox3D::RenderPBRTest()
{
    auto& camera =
        m_CameraController.GetCamera();

    m_PBRShader->Bind();

    // 相机、灯光和曝光由所有球体共享。
    m_PBRShader->SetMat4(
        "u_ViewProjection",
        camera.GetViewProjectionMatrix());

    // 当前 SetFloat3 接口接收非 const 引用。
    glm::vec3 cameraPosition =
        camera.GetPosition();

    m_PBRShader->SetFloat3(
        "u_CameraPosition",
        cameraPosition);

    m_PBRShader->SetFloat(
        "u_Exposure",
        m_PBRExposure);

    Hazel::LightingUploader::Upload(
        *m_PBRShader,
        m_PBRLighting);

    const auto& vertexArray =
        m_PBRSphere->GetVertexArray();

    vertexArray->Bind();

    for (const auto& object : m_PBRObjects)
    {
        // Binder 绑定的仍是同一个 PBR Shader，
        // 之前上传的共享 uniform 保持有效。
        Hazel::MaterialBinder::Bind(
            *object.Material,
            0,
            16);

        m_PBRShader->SetMat4(
            "u_Model",
            object.Transform);

        m_PBRShader->SetMat3(
            "u_NormalMatrix",
            object.NormalMatrix);

        Hazel::RenderCommand::DrawIndexed(
            vertexArray,
            m_PBRSphere->GetIndexCount());
    }
}

void Sandbox3D::RenderEnvironment()
{
    auto& camera =
        m_CameraController.GetCamera();

    m_EnvironmentShader->Bind();

    m_EnvironmentShader->SetMat4(
        "u_InverseProjection",
        glm::inverse(
            camera.GetProjectionMatrix()));

    m_EnvironmentShader->SetMat3(
        "u_InverseViewRotation",
        glm::transpose(
            glm::mat3(camera.GetViewMatrix())));

    m_EnvironmentShader->SetFloat(
        "u_EnvironmentRotation",
        glm::radians(m_EnvironmentRotation));

    m_EnvironmentShader->SetFloat(
        "u_Intensity",
        m_BackgroundIntensity);

    m_EnvironmentMap->Bind(0);

    m_EnvironmentShader->SetInt(
        "u_EnvironmentMap",
        0);

    // 背景是全屏四边形，不应继承网格的深度、混合和剔除状态。
    const GLboolean depthTest =
        glIsEnabled(GL_DEPTH_TEST);

    const GLboolean blend =
        glIsEnabled(GL_BLEND);

    const GLboolean cull =
        glIsEnabled(GL_CULL_FACE);

    GLboolean depthWrite = GL_TRUE;
    glGetBooleanv(
        GL_DEPTH_WRITEMASK,
        &depthWrite);

    glDisable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);
    glDisable(GL_BLEND);
    glDisable(GL_CULL_FACE);

    m_EnvironmentQuad->Bind();

    // 单个全屏三角形覆盖整个目标，不依赖索引缓冲。
    glDrawArrays(
        GL_TRIANGLES,
        0,
        3);

    glDepthMask(depthWrite);

    if (depthTest)
        glEnable(GL_DEPTH_TEST);

    if (blend)
        glEnable(GL_BLEND);

    if (cull)
        glEnable(GL_CULL_FACE);
}

void Sandbox3D::RenderModel()
{
    auto& camera =
        m_CameraController.GetCamera();

    // 为环境贴图保留两个纹理槽。
    const uint32_t slot =
        Hazel::MaterialBinder::Bind(
            *m_ModelMaterial,
            0,
            14);

    auto& shader =
        *m_ModelMaterial
            ->GetMaterial()
            ->GetShader();

    shader.SetMat4(
        "u_ViewProjection",
        camera.GetViewProjectionMatrix());

    glm::vec3 cameraPosition =
        camera.GetPosition();

    shader.SetFloat3(
        "u_CameraPosition",
        cameraPosition);

    Hazel::LightingUploader::Upload(
        shader,
        m_Lighting);

    m_EnvironmentMap->Bind(slot);
    m_IrradianceMap->Bind(slot + 1);

    shader.SetInt(
        "u_EnvironmentMap",
        static_cast<int>(slot));

    shader.SetInt(
        "u_IrradianceMap",
        static_cast<int>(slot + 1));

    shader.SetInt(
        "u_EnvironmentEnabled",
        m_EnvironmentLighting ? 1 : 0);

    shader.SetFloat(
        "u_EnvironmentIntensity",
        m_EnvironmentIntensity);

    shader.SetFloat(
        "u_ReflectionStrength",
        m_ReflectionStrength);

    shader.SetFloat(
        "u_EnvironmentRotation",
        glm::radians(m_EnvironmentRotation));

    shader.SetMat4(
        "u_Model",
        m_ModelTransform);

    shader.SetMat3(
        "u_NormalMatrix",
        m_NormalMatrix);

    const auto& vertexArray =
        m_Model->GetVertexArray();

    vertexArray->Bind();

    Hazel::RenderCommand::DrawIndexed(
        vertexArray,
        m_Model->GetIndexCount());
}

// ============================================================
// ImGui
// ============================================================

void Sandbox3D::OnImGuiRender()
{
    if (ImGui::Begin("Sandbox3D"))
    {
        if (ImGui::Checkbox(
                "Show PBR grid",
                &m_ShowPBRTest))
        {
            ResetTestCamera();
        }

        if (ImGui::Button("Reset camera"))
            ResetTestCamera();

        ImGui::TextUnformatted(
            "RMB: look | WASD: move | Q/E: down/up");

        ImGui::Separator();

        if (m_ShowPBRTest)
            OnPBRImGuiRender();
        else
            OnModelImGuiRender();
    }

    ImGui::End();
}

void Sandbox3D::OnPBRImGuiRender()
{
    ImGui::TextUnformatted(
        "Left -> Right: roughness 0.05 -> 1.0");

    ImGui::TextUnformatted(
        "Bottom -> Top: metallic 0.0 -> 1.0");

    if (ImGui::ColorEdit3(
            "Base color",
            glm::value_ptr(m_PBRBaseColor)))
    {
        for (auto& object : m_PBRObjects)
        {
            object.Material->Set(
                "u_Material.BaseColor",
                glm::vec4(
                    m_PBRBaseColor,
                    1.0f));
        }
    }

    ImGui::SliderFloat(
        "Exposure",
        &m_PBRExposure,
        0.1f,
        5.0f,
        "%.2f",
        ImGuiSliderFlags_AlwaysClamp);

    auto& sun =
        m_PBRLighting.DirectionalLights.front();

    ImGui::Checkbox(
        "Sun enabled",
        &sun.Properties.Enabled);

    ImGui::SliderFloat(
        "Sun intensity",
        &sun.Properties.Intensity,
        0.0f,
        10.0f,
        "%.2f",
        ImGuiSliderFlags_AlwaysClamp);

    float pointIntensity =
        m_PBRLighting.PointLights.front()
            .Properties.Intensity;

    if (ImGui::SliderFloat(
            "Point intensity",
            &pointIntensity,
            0.0f,
            300.0f,
            "%.1f",
            ImGuiSliderFlags_AlwaysClamp))
    {
        for (auto& light :
             m_PBRLighting.PointLights)
        {
            light.Properties.Intensity =
                pointIntensity;
        }
    }

    auto& spot =
        m_PBRLighting.SpotLights.front();

    ImGui::Checkbox(
        "Spot enabled",
        &spot.Properties.Enabled);

    ImGui::SliderFloat(
        "Spot intensity",
        &spot.Properties.Intensity,
        0.0f,
        300.0f,
        "%.1f",
        ImGuiSliderFlags_AlwaysClamp);

    ImGui::SliderFloat(
        "Debug ambient",
        &m_PBRLighting.Ambient.Intensity,
        0.0f,
        0.2f,
        "%.3f",
        ImGuiSliderFlags_AlwaysClamp);
}

void Sandbox3D::OnModelImGuiRender()
{
    if (!m_ModelMaterial)
    {
        ImGui::TextUnformatted(
            "OBJ model could not be loaded.");

        return;
    }

    ImGui::ColorEdit3(
        "Ambient color",
        glm::value_ptr(
            m_Lighting.Ambient.Color));

    ImGui::SliderFloat(
        "Ambient intensity",
        &m_Lighting.Ambient.Intensity,
        0.0f,
        1.0f,
        "%.2f",
        ImGuiSliderFlags_AlwaysClamp);

    ImGui::Checkbox(
        "Show background",
        &m_ShowEnvironment);

    ImGui::Checkbox(
        "Environment lighting",
        &m_EnvironmentLighting);

    ImGui::SliderFloat(
        "Diffuse intensity",
        &m_EnvironmentIntensity,
        0.0f,
        2.0f,
        "%.2f",
        ImGuiSliderFlags_AlwaysClamp);

    ImGui::SliderFloat(
        "Reflection strength",
        &m_ReflectionStrength,
        0.0f,
        1.0f,
        "%.2f",
        ImGuiSliderFlags_AlwaysClamp);

    ImGui::SliderFloat(
        "Background intensity",
        &m_BackgroundIntensity,
        0.0f,
        2.0f,
        "%.2f",
        ImGuiSliderFlags_AlwaysClamp);

    ImGui::SliderFloat(
        "Environment rotation",
        &m_EnvironmentRotation,
        -180.0f,
        180.0f,
        "%.0f deg",
        ImGuiSliderFlags_AlwaysClamp);
}

// ============================================================
// 事件与资源释放
// ============================================================

void Sandbox3D::OnEvent(Hazel::Event& event)
{
    if (event.GetEventType() ==
        Hazel::EventType::WindowResize)
    {
        const auto& resize =
            static_cast<const Hazel::WindowResizeEvent&>(
                event);

        // 避免当前控制器用零高度计算投影比例。
        if (resize.GetWidth() > 0 &&
            resize.GetHeight() > 0)
        {
            m_CameraController.OnEvent(event);
        }

        return;
    }

    const auto& io =
        ImGui::GetIO();

    if (
        (io.WantCaptureMouse &&
         event.IsInCategory(
             Hazel::EventCategoryMouse)) ||

        (io.WantCaptureKeyboard &&
         event.IsInCategory(
             Hazel::EventCategoryKeyboard))
    )
    {
        return;
    }

    m_CameraController.OnEvent(event);
}

void Sandbox3D::OnDetach()
{
    m_PBRSceneEntities.clear();
    m_PBRScenePointLights.clear();

    m_PBRSceneDirectionalLight = {};
    m_PBRSceneSpotLight = {};
    m_ModelSceneEntity = {};
    m_ModelSceneDirectionalLight = {};
    m_ModelScenePointLight = {};
    m_ModelSceneSpotLight = {};

    m_TransformSystem.Clear();
    m_RenderSceneData.Clear();
    m_Scene.reset();

    m_PBRObjects.clear();
    m_PBRSphere.reset();
    m_PBRShader.reset();

    m_ModelMaterial.reset();
    m_ModelShader.reset();
    m_Model.reset();

    m_EnvironmentQuad.reset();
    m_EnvironmentShader.reset();

    m_IrradianceMap.reset();
    m_EnvironmentMap.reset();

    m_MaterialSystem.reset();
}
