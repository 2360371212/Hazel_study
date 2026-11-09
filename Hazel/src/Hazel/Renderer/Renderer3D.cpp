#include "hzpch.h"
#include "Renderer3D.h"

#include <glm/gtc/matrix_inverse.hpp>

#include <glad/glad.h>

#include "Hazel/Renderer/Common/RenderCommand.h"
#include "Hazel/Renderer/Common/Shader.h"
#include "Hazel/Renderer/Geometry/Mesh.h"
#include "Hazel/Renderer/Lighting/LightingUploader.h"
#include "Hazel/Renderer/Material/MaterialBinder.h"

namespace Hazel
{
    namespace
    {
        constexpr uint32_t MaterialTextureSlotLimit = 14;
        constexpr uint32_t EnvironmentTextureSlotCount = 2;
    }

    void Renderer3D::Render(const RenderSceneData& scene, const RenderView& view, const Renderer3DSettings& settings)
    {
        RenderCommand::SetViewport(0, 0, view.ViewportWidth, view.ViewportHeight);

        if (settings.ClearTarget)
        {
            RenderCommand::SetClearColor(scene.Environment.ClearColor);
            RenderCommand::Clear();
        }

        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LESS);
        glDepthMask(GL_TRUE);

        // 第一版先关闭混合，透明物体后续单独排序处理。
        glDisable(GL_BLEND);

        for (const RenderMeshData& meshData : scene.Meshes)
        {
            if (!meshData.MeshResource || !meshData.Material)
            {
                continue;
            }
            const RenderState& renderState = meshData.Material->GetRenderState();

            ApplyRenderState(renderState);

            const uint32_t nextTextureSlot = MaterialBinder::Bind(*meshData.Material, 0, MaterialTextureSlotLimit);

            Ref<Shader> shader = meshData.Material->GetMaterial()->GetShader();

            if (!shader)
                continue;

            UploadFrameParameters(*shader, scene, view, settings);

            shader->SetMat4("u_Model", meshData.WorldMatrix);

            const glm::mat3 normalMatrix =glm::inverseTranspose(glm::mat3(meshData.WorldMatrix));

            shader->SetMat3("u_NormalMatrix", normalMatrix);

            const auto& environmentMap = scene.Environment.EnvironmentMap;

            const auto& irradianceMap = scene.Environment.IrradianceMap;

            const bool hasEnvironment = environmentMap && irradianceMap;

            if (hasEnvironment)
            {
                environmentMap->Bind(nextTextureSlot);

                irradianceMap->Bind(nextTextureSlot + 1);

                shader->SetInt("u_EnvironmentMap", static_cast<int>(nextTextureSlot));

                shader->SetInt("u_IrradianceMap", static_cast<int>(nextTextureSlot + 1));
            }

            shader->SetInt("u_EnvironmentEnabled", scene.Environment.EnvironmentLightingEnabled && hasEnvironment ? 1 : 0);
            shader->SetFloat("u_EnvironmentIntensity", scene.Environment.EnvironmentIntensity);
            shader->SetFloat("u_ReflectionStrength", settings.ReflectionStrength);
            shader->SetFloat("u_EnvironmentRotation", scene.Environment.RotationRadians);

            const auto& vertexArray = meshData.MeshResource->GetVertexArray();

            if (!vertexArray)
                continue;

            vertexArray->Bind();

            RenderCommand::DrawIndexed(vertexArray, meshData.MeshResource->GetIndexCount());
        }

        // 防止后续 2D 渲染继承透明状态。
        glDisable(GL_BLEND);
        glEnable(GL_DEPTH_TEST);
        glDepthMask(GL_TRUE);
    }

    void Renderer3D::ApplyRenderState(const RenderState& state)
    {
        if (state.DepthTest)
            glEnable(GL_DEPTH_TEST);
        else
            glDisable(GL_DEPTH_TEST);

        glDepthMask(state.DepthWrite ? GL_TRUE : GL_FALSE);

        switch (state.Cull)
        {
            case CullMode::None:
                glDisable(GL_CULL_FACE);
                break;

            case CullMode::Front:
                glEnable(GL_CULL_FACE);
                glCullFace(GL_FRONT);
                break;

            case CullMode::Back:
                glEnable(GL_CULL_FACE);
                glCullFace(GL_BACK);
                break;
        }

        switch (state.Blend)
        {
            case BlendMode::Opaque:
                glDisable(GL_BLEND);
                break;

            case BlendMode::AlphaBlend:
                glEnable(GL_BLEND);
                glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
                break;

            case BlendMode::Additive:
                glEnable(GL_BLEND);
                glBlendFunc(GL_SRC_ALPHA, GL_ONE);
                break;
        }
    }

    void Renderer3D::UploadFrameParameters(Shader& shader, const RenderSceneData& scene, const RenderView& view, const Renderer3DSettings& settings)
    {
        shader.SetMat4("u_ViewProjection", view.ViewProjection);

        glm::vec3 cameraPosition = view.Position;
        shader.SetFloat3("u_CameraPosition", cameraPosition);
        shader.SetFloat("u_Exposure", settings.Exposure);
        LightingUploader::Upload(shader, scene.Lighting);
    }
}