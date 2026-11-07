#pragma once

#include <filesystem>
#include <memory>
#include <vector>

#include "Hazel.h"

#include "Hazel/Renderer/Material/MaterialSystem.h"
#include "Hazel/Renderer/Material/MaterialInstance.h"
#include "Hazel/Renderer/Material/MaterialBinder.h"

#include "Hazel/Renderer/Lighting/SceneLighting.h"
#include "Hazel/Renderer/Lighting/LightingUploader.h"

class Sandbox3D : public Hazel::Layer
{
public:
    Sandbox3D();

    void OnAttach() override;
    void OnDetach() override;

    void OnUpdate(Hazel::Timestep timestep) override;
    void OnImGuiRender() override;
    void OnEvent(Hazel::Event& event) override;

private:
    struct PBRTestObject
    {
        Hazel::Ref<Hazel::MaterialInstance> Material;

        glm::mat4 Transform{ 1.0f };
        glm::mat3 NormalMatrix{ 1.0f };
    };

private:
    void CreatePBRTest(
        const std::filesystem::path& engineAssetsRoot);

    void CreateModelTest(
        const std::filesystem::path& sandboxAssetsRoot);

    void CreateEnvironment(
        const std::filesystem::path& sandboxAssetsRoot);

    void RenderPBRTest();
    void RenderModel();
    void RenderEnvironment();

    // 在 OnImGuiRender 创建的窗口内绘制控件。
    void OnPBRImGuiRender();
    void OnModelImGuiRender();

    void ResetTestCamera();

private:
    Hazel::FlyCameraController m_CameraController;

    Hazel::Scope<Hazel::MaterialSystem> m_MaterialSystem;

    // PBR 球体测试。
    bool m_ShowPBRTest = true;

    Hazel::Ref<Hazel::Mesh> m_PBRSphere;
    Hazel::Ref<Hazel::Shader> m_PBRShader;

    std::vector<PBRTestObject> m_PBRObjects;

    Hazel::SceneLighting m_PBRLighting;

    glm::vec3 m_PBRBaseColor{
        0.8f,
        0.2f,
        0.08f
    };

    float m_PBRExposure = 1.2f;

    // 原有模型测试。
    Hazel::Ref<Hazel::Mesh> m_Model;
    Hazel::Ref<Hazel::Shader> m_ModelShader;

    Hazel::Ref<Hazel::MaterialInstance> m_ModelMaterial;

    Hazel::SceneLighting m_Lighting;

    glm::mat4 m_ModelTransform{ 1.0f };
    glm::mat3 m_NormalMatrix{ 1.0f };

    // 原有环境测试。
    Hazel::Ref<Hazel::Texture2D> m_EnvironmentMap;
    Hazel::Ref<Hazel::Texture2D> m_IrradianceMap;

    Hazel::Ref<Hazel::Shader> m_EnvironmentShader;
    Hazel::Ref<Hazel::VertexArray> m_EnvironmentQuad;

    bool m_ShowEnvironment = true;
    bool m_EnvironmentLighting = true;

    float m_EnvironmentIntensity = 0.7f;
    float m_ReflectionStrength = 0.2f;
    float m_BackgroundIntensity = 0.8f;
    float m_EnvironmentRotation = 0.0f;
};