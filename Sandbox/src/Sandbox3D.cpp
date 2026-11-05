#include "Sandbox3D.h"

#include <algorithm>
#include <cstdint>
#include <exception>
#include <filesystem>
#include <fstream>
#include <limits>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

// #include <glad/glad.h>

#include <glm/gtc/matrix_transform.hpp>

namespace
{
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

        std::vector<glm::vec3> positions;
        std::vector<glm::vec2> texCoords;
        std::vector<glm::vec3> normals;

        std::unordered_map<
            ObjVertexKey,
            uint32_t,
            ObjVertexKeyHash>
            vertexCache;

        bool hasExplicitNormals = false;

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
                    ObjVertexKey key;

                    if (!ParseOBJVertexToken(
                            token,
                            key))
                    {
                        HZ_CORE_ERROR(
                            "Invalid OBJ face at line {}",
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

                    const int positionIndex =
                        ResolveOBJIndex(
                            key.Position,
                            positions.size());

                    if (positionIndex < 0 ||
                        positionIndex >=
                        static_cast<int>(
                            positions.size()))
                    {
                        HZ_CORE_ERROR(
                            "OBJ position index out of range at line {}",
                            lineNumber);

                        return false;
                    }

                    Hazel::MeshVertex vertex;

                    vertex.Position =
                        positions[positionIndex];

                    if (key.TexCoord != 0)
                    {
                        const int texCoordIndex =
                            ResolveOBJIndex(
                                key.TexCoord,
                                texCoords.size());

                        if (texCoordIndex >= 0 &&
                            texCoordIndex <
                            static_cast<int>(
                                texCoords.size()))
                        {
                            vertex.TexCoord =
                                texCoords[texCoordIndex];

                            // OpenGL UV 通常将 V 轴翻转。
                            vertex.TexCoord.y =
                                1.0f -
                                vertex.TexCoord.y;
                        }
                    }

                    if (key.Normal != 0)
                    {
                        const int normalIndex =
                            ResolveOBJIndex(
                                key.Normal,
                                normals.size());

                        if (normalIndex >= 0 &&
                            normalIndex <
                            static_cast<int>(
                                normals.size()))
                        {
                            vertex.Normal =
                                normals[normalIndex];

                            hasExplicitNormals = true;
                        }
                    }

                    const uint32_t vertexIndex =
                        static_cast<uint32_t>(
                            meshData.Vertices.size());

                    meshData.Vertices.push_back(
                        vertex);

                    vertexCache.emplace(
                        key,
                        vertexIndex);

                    faceIndices.push_back(
                        vertexIndex);
                }

                // 将 OBJ 多边形面转换为三角形。
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

        // OBJ 没有法线时，根据三角形计算法线。
        if (!hasExplicitNormals)
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

                a.Normal += normal;
                b.Normal += normal;
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

    Hazel::AABB TransformAABB(
        const Hazel::AABB& localBounds,
        const glm::mat4& transform)
    {
        const glm::vec3 corners[8] = {
            {
                localBounds.Min.x,
                localBounds.Min.y,
                localBounds.Min.z
            },
            {
                localBounds.Max.x,
                localBounds.Min.y,
                localBounds.Min.z
            },
            {
                localBounds.Max.x,
                localBounds.Max.y,
                localBounds.Min.z
            },
            {
                localBounds.Min.x,
                localBounds.Max.y,
                localBounds.Min.z
            },
            {
                localBounds.Min.x,
                localBounds.Min.y,
                localBounds.Max.z
            },
            {
                localBounds.Max.x,
                localBounds.Min.y,
                localBounds.Max.z
            },
            {
                localBounds.Max.x,
                localBounds.Max.y,
                localBounds.Max.z
            },
            {
                localBounds.Min.x,
                localBounds.Max.y,
                localBounds.Max.z
            }
        };

        Hazel::AABB worldBounds;

        worldBounds.Min =
            glm::vec3(
                std::numeric_limits<float>::max());

        worldBounds.Max =
            glm::vec3(
                std::numeric_limits<float>::lowest());

        for (const glm::vec3& corner :
             corners)
        {
            const glm::vec3 worldPosition =
                glm::vec3(
                    transform *
                    glm::vec4(
                        corner,
                        1.0f));

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

    const std::filesystem::path modelPath =
        "D:/Project/cpp/Hazel/Sandbox/assets/model/Intergalactic_Spaceship-(Wavefront).obj";

    Hazel::MeshData modelData;

    if (!LoadOBJ(
            modelPath,
            modelData))
    {
        HZ_CORE_ERROR(
            "Failed to load model: {}",
            modelPath.string());

        return;
    }

    m_Model =
        Hazel::Mesh::Create(modelData);

    if (!m_Model)
    {
        HZ_CORE_ERROR(
            "Failed to create Mesh from OBJ: {}",
            modelPath.string());

        return;
    }

    const Hazel::AABB& bounds =
        m_Model->GetBounds();

    const glm::vec3 center =
        (bounds.Min + bounds.Max) *
        0.5f;

    const glm::vec3 size =
        bounds.Max - bounds.Min;

    const float largestExtent =
        std::max(
            size.x,
            std::max(size.y, size.z));

    const float scale =
        largestExtent > 0.0001f
            ? 3.0f / largestExtent
            : 1.0f;

    m_ModelTransform =
        glm::scale(
            glm::mat4(1.0f),
            glm::vec3(scale)) *
        glm::translate(
            glm::mat4(1.0f),
            -center);

    const std::string vertexSource = R"(
        #version 330 core

        layout(location = 0) in vec3 a_Position;
        layout(location = 1) in vec3 a_Normal;
        layout(location = 2) in vec2 a_TexCoord;

        uniform mat4 u_ViewProjection;
        uniform mat4 u_Transform;

        out vec3 v_Normal;

        void main()
        {
            v_Normal = a_Normal;

            gl_Position =
                u_ViewProjection *
                u_Transform *
                vec4(a_Position, 1.0);
        }
    )";

    const std::string fragmentSource = R"(
        #version 330 core

        layout(location = 0) out vec4 o_Color;

        in vec3 v_Normal;

        uniform vec4 u_Tint;

        void main()
        {
            vec3 normal =
                normalize(v_Normal);

            vec3 lightDirection =
                normalize(vec3(0.4, 0.8, 0.5));

            float diffuse =
                max(
                    dot(normal, lightDirection),
                    0.0);

            vec3 lighting =
                vec3(0.25) +
                diffuse * vec3(0.75);

            o_Color =
                vec4(
                    u_Tint.rgb * lighting,
                    u_Tint.a);
        }
    )";

    m_ModelShader =
        Hazel::Shader::Create(
            "Sandbox3D_Model",
            vertexSource,
            fragmentSource);

    CreateBoundsDebugGeometry();

    HZ_CORE_INFO(
        "Loaded OBJ model: {} vertices, {} indices",
        m_Model->GetVertexCount(),
        m_Model->GetIndexCount());
}

void Sandbox3D::CreateBoundsDebugGeometry()
{
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
        0, 1, 2,
        2, 3, 0,

        4, 5, 6,
        6, 7, 4,

        0, 4, 7,
        7, 3, 0,

        1, 2, 6,
        6, 5, 1,

        0, 1, 5,
        5, 4, 0,

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

    // glDisable(GL_DEPTH_TEST);

    // glPolygonMode(
    //     GL_FRONT_AND_BACK,
    //     GL_LINE);

    // glLineWidth(2.0f);

    // Hazel::RenderCommand::DrawIndexed(
    //     m_BoundsVertexArray,
    //     36);

    // glPolygonMode(
    //     GL_FRONT_AND_BACK,
    //     GL_FILL);

    // glEnable(GL_DEPTH_TEST);
}

void Sandbox3D::OnDetach()
{
    m_BoundsShader.reset();
    m_BoundsVertexArray.reset();

    m_ModelShader.reset();
    m_Model.reset();
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

    if (!m_Model ||
        !m_ModelShader)
    {
        return;
    }

    auto& camera =
        m_CameraController.GetCamera();

    m_ModelShader->Bind();

    m_ModelShader->SetMat4(
        "u_ViewProjection",
        camera.GetViewProjectionMatrix());

    m_ModelShader->SetMat4(
        "u_Transform",
        m_ModelTransform);

    m_ModelShader->SetFloat4(
        "u_Tint",
        glm::vec4(
            0.65f,
            0.72f,
            0.86f,
            1.0f));

    const auto& vertexArray =
        m_Model->GetVertexArray();

    vertexArray->Bind();

    Hazel::RenderCommand::DrawIndexed(
        vertexArray,
        m_Model->GetIndexCount());

    DrawBounds(
        m_Model->GetBounds(),
        m_ModelTransform);
}

void Sandbox3D::OnEvent(
    Hazel::Event& event)
{
    m_CameraController.OnEvent(
        event);
}