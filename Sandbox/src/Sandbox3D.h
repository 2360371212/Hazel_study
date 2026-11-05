#pragma once

#include "Hazel.h"

class Sandbox3D : public Hazel::Layer
{
public:
    Sandbox3D();

    void OnAttach() override;
    void OnDetach() override;

    void OnUpdate(Hazel::Timestep timestep) override;
    void OnEvent(Hazel::Event& event) override;

private:
    void CreateBoundsDebugGeometry();

    void DrawBounds(
        const Hazel::AABB& localBounds,
        const glm::mat4& transform);

private:
    Hazel::FlyCameraController m_CameraController;

    Hazel::Ref<Hazel::Mesh> m_Cube;
    Hazel::Ref<Hazel::Shader> m_CubeShader;

    Hazel::Ref<Hazel::VertexArray> m_BoundsVertexArray;
    Hazel::Ref<Hazel::Shader> m_BoundsShader;
};