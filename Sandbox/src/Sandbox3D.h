#pragma once

#include <cstdint>
#include <memory>

#include "Hazel.h"

#include "Hazel/Renderer/Pipeline/RenderPipeline.h"
#include "Hazel/Renderer/Scene/RenderSceneData.h"

#include "Hazel/Scene/Scene.h"
#include "Hazel/Scene/Systems/TransformSystem.h"

class Sandbox3D final : public Hazel::Layer
{
public:
    Sandbox3D();

    void OnAttach() override;
    void OnDetach() override;

    void OnUpdate(Hazel::Timestep timestep) override;
    void OnImGuiRender() override;
    void OnEvent(Hazel::Event& event) override;

private:
    void ResetCamera();

private:
    Hazel::FlyCameraController m_CameraController;

    Hazel::Scope<Hazel::Scene> m_Scene;
    Hazel::TransformSystem m_TransformSystem;
    Hazel::RenderSceneData m_RenderSceneData;

    Hazel::Scope<Hazel::RenderPipeline> m_Pipeline;

    uint32_t m_ViewportWidth = 0;
    uint32_t m_ViewportHeight = 0;
};