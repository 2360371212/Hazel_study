#pragma once

#include <vector>

#include <glm/glm.hpp>

#include "Hazel/Core/Core.h"
#include "Hazel/Core/UUID.h"
#include "Hazel/Renderer/Geometry/MeshTypes.h"
#include "Hazel/Renderer/Lighting/SceneLighting.h"
#include "Hazel/Renderer/Common/Texture.h"

namespace Hazel
{
    class Mesh;
    class MaterialInstance;
    // 一个待渲染的网格对象。
    struct RenderMeshData
    {
        // 保留来源标识，方便调试、选中和后续拾取。
        UUID EntityID{ 0 };

        Ref<Mesh> MeshResource;
        Ref<MaterialInstance> Material;

        glm::mat4 WorldMatrix{ 1.0f };

        // 已转换到世界空间的轴对齐包围盒。
        AABB WorldBounds;
    };

    // 渲染端需要的环境配置。
    // 常量环境光保存在 SceneLighting::Ambient 中。
    struct RenderEnvironmentData
    {
        glm::vec4 ClearColor{
            0.035f,
            0.045f,
            0.065f,
            1.0f
        };

        Ref<Texture2D> EnvironmentMap;
        Ref<Texture2D> IrradianceMap;
        Ref<TextureCube> SkyboxMap;

        bool ShowBackground = true;
        bool EnvironmentLightingEnabled = false;

        float EnvironmentIntensity = 1.0f;
        float BackgroundIntensity = 1.0f;

        float RotationRadians = 0.0f;
        float Exposure = 1.0f;
    };

    // 一次场景提取的结果。
    struct RenderSceneData
    {
        std::vector<RenderMeshData> Meshes;

        SceneLighting Lighting;
        RenderEnvironmentData Environment;

        void Clear()
        {
            // clear 保留 vector 容量，供后续帧复用。
            Meshes.clear();

            Lighting.DirectionalLights.clear();
            Lighting.PointLights.clear();
            Lighting.SpotLights.clear();

            Lighting.Ambient = AmbientLight{};
            Environment = RenderEnvironmentData{};
        }
    };
}
