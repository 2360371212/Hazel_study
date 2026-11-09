#include "hzpch.h"
#include "SceneRenderExtractor.h"

#include "Hazel/Scene/Scene.h"
#include "Hazel/Scene/Components/TransformComponent.h"
#include "Hazel/Scene/Components/MeshRendererComponent.h"
#include "Hazel/Scene/Components/LightComponents.h"
#include "Hazel/Scene/Systems/TransformSystem.h"

#include "Hazel/Renderer/Geometry/Mesh.h"
#include "Hazel/Renderer/Scene/RenderSceneData.h"

namespace Hazel
{
    namespace
    {
        // 将局部 AABB 转换成包住变换后物体的世界 AABB。
        // worldMatrix 为通常的仿射模型变换矩阵。
        AABB TransformBounds(const AABB& localBounds, const glm::mat4& worldMatrix)
        {
            const glm::vec3 localCenter = (localBounds.Min + localBounds.Max) * 0.5f;
            const glm::vec3 localHalfExtent = (localBounds.Max - localBounds.Min) * 0.5f;
            const glm::vec3 worldCenter = glm::vec3(worldMatrix * glm::vec4(localCenter, 1.0f));

            // 三个基向量分别贡献变换后的半尺寸。
            // 取绝对值，可正确处理旋转与负缩放。
            const glm::vec3 worldHalfExtent =
                glm::abs(glm::vec3(worldMatrix[0]))
                    * localHalfExtent.x
                + glm::abs(glm::vec3(worldMatrix[1]))
                    * localHalfExtent.y
                + glm::abs(glm::vec3(worldMatrix[2]))
                    * localHalfExtent.z;

            AABB result;
            result.Min = worldCenter - worldHalfExtent;
            result.Max = worldCenter + worldHalfExtent;

            return result;
        }
    }

    void SceneRenderExtractor::Extract(const Scene& scene, const TransformSystem& transforms, RenderSceneData& output)
    {
        output.Clear();

        ExtractEnvironment(scene, output);
        ExtractMeshes(scene, transforms, output);
        ExtractLights(scene, transforms, output);
    }

    void SceneRenderExtractor::ExtractMeshes(const Scene& scene, const TransformSystem& transforms, RenderSceneData& output)
    {
        scene.ForEach<MeshRendererComponent>([&](UUID id, const MeshRendererComponent& component)
            {
                if (!scene.IsEntityEnabled(id) || !component.Visible)
                    return;

                // 允许编辑器中存在尚未配置资源的组件。
                if (!component.MeshResource || !component.Material)
                    return;

                // 当前 Scene 不会自动为实体添加 Transform。
                if (!scene.HasComponent<TransformComponent>(id))
                    return;

                const auto& world = transforms.GetWorldTransform(id);
                auto& item = output.Meshes.emplace_back();

                item.EntityID = id;
                item.MeshResource = component.MeshResource;
                item.Material = component.Material;
                item.WorldMatrix = world.Matrix;

                item.WorldBounds = TransformBounds(component.MeshResource->GetBounds(), world.Matrix);
            });
    }

    void SceneRenderExtractor::ExtractLights(const Scene& scene, const TransformSystem& transforms, RenderSceneData& output)
    {
        // 方向光。
        scene.ForEach<DirectionalLightComponent>(
            [&](UUID id, const DirectionalLightComponent& component)
            {
                if (!scene.IsEntityEnabled(id) || !component.Properties.Enabled)
                {
                    return;
                }

                if (!scene.HasComponent<TransformComponent>(id))
                    return;

                const auto& world = transforms.GetWorldTransform(id);
                auto& light = output.Lighting.DirectionalLights.emplace_back();
                light.Properties = component.Properties;

                // 方向表示光线传播方向。
                light.Direction = world.GetForward();
            });

        // 点光源。
        scene.ForEach<PointLightComponent>(
            [&](UUID id, const PointLightComponent& component)
            {
                if (!scene.IsEntityEnabled(id) || !component.Properties.Enabled)
                {
                    return;
                }

                if (!scene.HasComponent<TransformComponent>(id))
                    return;

                const auto& world = transforms.GetWorldTransform(id);

                auto& light = output.Lighting.PointLights.emplace_back();

                light.Properties = component.Properties;
                light.Position = world.Position;
                light.Range = component.Range;
            });

        // 聚光灯。
        scene.ForEach<SpotLightComponent>(
            [&](UUID id, const SpotLightComponent& component)
            {
                if (!scene.IsEntityEnabled(id)
                    || !component.Properties.Enabled)
                {
                    return;
                }

                if (!scene.HasComponent<TransformComponent>(id))
                    return;

                const auto& world =
                    transforms.GetWorldTransform(id);

                auto& light =
                    output.Lighting.SpotLights.emplace_back();

                light.Properties = component.Properties;

                light.Position = world.Position;
                light.Direction = world.GetForward();

                light.Range = component.Range;

                // 保持现有约定：CPU 侧使用度数。
                // LightingUploader 再转换成余弦值。
                light.InnerConeAngle = component.InnerConeAngle;
                light.OuterConeAngle = component.OuterConeAngle;
            });
    }

    void SceneRenderExtractor::ExtractEnvironment(const Scene& scene, RenderSceneData& output)
    {
        const auto& source = scene.GetEnvironment();

        output.Lighting.Ambient.Color = source.AmbientColor;
        output.Lighting.Ambient.Intensity = source.AmbientIntensity;

        auto& target = output.Environment;

        target.ClearColor = source.ClearColor;

        target.EnvironmentMap = source.EnvironmentMap;
        target.IrradianceMap = source.IrradianceMap;
        target.SkyboxMap = source.SkyboxMap;
        target.ShowBackground = source.ShowBackground;
        target.EnvironmentLightingEnabled = source.EnvironmentLightingEnabled;
        target.EnvironmentIntensity = source.EnvironmentIntensity;
        target.BackgroundIntensity = source.BackgroundIntensity;
        target.RotationRadians = source.RotationRadians;
        target.Exposure = source.Exposure;
    }
}
