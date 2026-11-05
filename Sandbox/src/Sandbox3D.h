#pragma once

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
    void RenderModel();
    void CreateEnvironment();
    void RenderEnvironment();

private:
    Hazel::FlyCameraController m_CameraController;

    Hazel::Ref<Hazel::Mesh> m_Model;
    Hazel::Ref<Hazel::Shader> m_ModelShader;

    Hazel::Scope<Hazel::MaterialSystem> m_MaterialSystem;
    Hazel::Ref<Hazel::MaterialInstance> m_ModelMaterial;

    Hazel::SceneLighting m_Lighting;

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

    glm::mat4 m_ModelTransform{ 1.0f };
    glm::mat3 m_NormalMatrix{ 1.0f };
};
