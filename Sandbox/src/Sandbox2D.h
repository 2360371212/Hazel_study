#pragma once

#include "Hazel.h"

class Sandbox2D : public Hazel::Layer{
    public:
        Sandbox2D();
        virtual ~Sandbox2D() = default;

        virtual void OnAttach() override;
        virtual void OnDetach() override;
        virtual void OnUpdate(Hazel::Timestep deltaTime) override;

        virtual void OnImGuiRender() override;
        virtual void OnEvent(Hazel::Event& event) override;

    private:
        Hazel::OrthographicCameraController m_CameraController;

        Hazel::Ref<Hazel::VertexArray> m_SquareVA;
        Hazel::Ref<Hazel::Shader> m_FlatColorShader;
        Hazel::Ref<Hazel::Texture2D> m_CheckerboardTexture;

        glm::vec4 m_SquareColor = {0.2f, 0.7f, 0.8f, 1.0f};

        struct ProfileResult{
            const char* Name;
            float Time;
        };

        std::vector<ProfileResult> m_ProfileResults;

};