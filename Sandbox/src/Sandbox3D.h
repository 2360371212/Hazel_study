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
    Hazel::FlyCameraController m_CameraController;
    Hazel::Ref<Hazel::VertexArray> m_CubeVertexArray;
    Hazel::Ref<Hazel::Shader> m_CubeShader;
};