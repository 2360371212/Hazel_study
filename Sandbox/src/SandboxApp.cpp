#include "Hazel.h"

#include "imgui.h"
#include "glm/gtc/matrix_transform.hpp"

class ExampleLayer : public Hazel::Layer{
    public:
        ExampleLayer()
            : Layer("Example"), m_Camera(-1.6f, 1.6f, -0.9f, 0.9f ), m_CameraPosition({0.0f, 0.0f, 0.0f}), m_SquarePosition({0.0f, 0.0f, 0.0f}){
            
            vertexArray.reset(Hazel::VertexArray::Create());

            float trangleVertices[3 * 7] = {
                -0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f,
                0.5f, -0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f,
                0.0f, 0.5f, 0.0f , 1.0f, 1.0f, 0.0f, 1.0f,
            };

            std::shared_ptr<Hazel::VertexBuffer> trangleVertexBuffer;
            trangleVertexBuffer.reset(Hazel::VertexBuffer::Create(trangleVertices, sizeof(trangleVertices)));
            Hazel::BufferLayout trangleLayout = {
                {Hazel::ShaderDataType::Float3, "a_Position"},
                {Hazel::ShaderDataType::Float4, "a_Color"}
            };
            trangleVertexBuffer->SetLayout(trangleLayout);
            
            uint32_t trangleIndices[3] = {0, 1, 2};
            std::shared_ptr<Hazel::IndexBuffer> trangleIndexBuffer;
            trangleIndexBuffer.reset(Hazel::IndexBuffer::Create(trangleIndices, sizeof(trangleIndices)/ sizeof(uint32_t)));

            vertexArray->AddVertexBuffer(trangleVertexBuffer);
            vertexArray->SetIndexBuffer(trangleIndexBuffer);

            std::string vertexSrc = R"(
                #version 330 core      

                layout(location = 0) in vec3 a_Position;
                layout(location = 1) in vec4 a_Color;

                uniform mat4 u_ViewProjection;
                uniform mat4 u_Transform;

                out vec3 v_Position;
                out vec4 v_Color;

                void main()
                {
                    v_Position = a_Position;
                    v_Color = a_Color;
                    gl_Position = u_ViewProjection * u_Transform * vec4(a_Position, 1.0);
                }
            )";

            std::string fragmentSrc= R"(
                #version 330 core      

                layout(location = 0) out vec4 color;

                in vec3 v_Position;

                in vec4 v_Color;

                void main()
                {
                    color = vec4(v_Position * 0.5 + 0.5, 1.0);
                    color = v_Color;
                }
            )";

            trangleShader.reset(new Hazel::Shader(vertexSrc, fragmentSrc));

            squareVertexArray.reset(Hazel::VertexArray::Create());

            float squareVertices[4 * 3] = {
                -0.75f, -0.75f, 0.0f,
                0.75f, -0.75f, 0.0f,
                0.75f, 0.75f, 0.0f,
                -0.75f, 0.75f, 0.0f
            };

            
            std::shared_ptr<Hazel::VertexBuffer> squareVertexBuffer;
            squareVertexBuffer.reset(Hazel::VertexBuffer::Create(squareVertices, sizeof(squareVertices)));
            Hazel::BufferLayout squareLayout= {
                {Hazel::ShaderDataType::Float3, "a_Position"},
            };
            squareVertexBuffer->SetLayout(squareLayout);
            
            uint32_t squareIndices[6] = {0, 1, 2, 2, 3, 0};
            std::shared_ptr<Hazel::IndexBuffer> squareIndexBuffer;
            squareIndexBuffer.reset(Hazel::IndexBuffer::Create(squareIndices, sizeof(squareIndices)/ sizeof(uint32_t)));

            squareVertexArray->AddVertexBuffer(squareVertexBuffer);
            squareVertexArray->SetIndexBuffer(squareIndexBuffer);


            std::string vertexSrc2 = R"(
                #version 330 core      

                layout(location = 0) in vec3 a_Position;

                uniform mat4 u_ViewProjection;
                uniform mat4 u_Transform;

                out vec3 v_Position;

                void main()
                {
                    v_Position = a_Position;
                    gl_Position = u_ViewProjection * u_Transform * vec4(a_Position, 1.0);
                }
            )";

            std::string fragmentSrc2 = R"(
                #version 330 core      

                layout(location = 0) out vec4 color;

                in vec3 v_Position;

                void main()
                {
                    color = vec4(0.2, 0.3, 0.8 ,1.0);
                }
            )";

            squareShader.reset(new Hazel::Shader(vertexSrc2,fragmentSrc2));
        }
        
        void OnUpdate(Hazel::Timestep deltaTime) override{


            if (Hazel::Input::IsKeyPressed(HZ_KEY_LEFT)){
                m_CameraPosition.x -= m_CameraMoveSpeed * deltaTime;
            } else if (Hazel::Input::IsKeyPressed(HZ_KEY_RIGHT)){
                m_CameraPosition.x += m_CameraMoveSpeed * deltaTime;
            }
            if (Hazel::Input::IsKeyPressed(HZ_KEY_UP)){
                m_CameraPosition.y += m_CameraMoveSpeed * deltaTime;
            } else if (Hazel::Input::IsKeyPressed(HZ_KEY_DOWN)){
                m_CameraPosition.y -= m_CameraMoveSpeed * deltaTime;
            }

            if (Hazel::Input::IsKeyPressed(HZ_KEY_A)){
                m_CameraRotation -= m_CameraRotationSpeed * deltaTime;
            }

            if (Hazel::Input::IsKeyPressed(HZ_KEY_D)){
                m_CameraRotation += m_CameraRotationSpeed * deltaTime;
            }

            if (Hazel::Input::IsKeyPressed(HZ_KEY_J)){
                m_SquarePosition.x -= m_SquareMoveSpeed * deltaTime;
            } else if (Hazel::Input::IsKeyPressed(HZ_KEY_L)){
                m_SquarePosition.x += m_SquareMoveSpeed * deltaTime;
            }
            if (Hazel::Input::IsKeyPressed(HZ_KEY_I)){
                m_SquarePosition.y += m_SquareMoveSpeed * deltaTime;
            } else if (Hazel::Input::IsKeyPressed(HZ_KEY_K)){
                m_SquarePosition.y -= m_SquareMoveSpeed * deltaTime;
            }




            Hazel::RenderCommand::SetClearColor({0.1f, 0.1f, 0.1f, 1});
            Hazel::RenderCommand::Clear();
            m_Camera.SetPosition(m_CameraPosition);
            m_Camera.SetRotation(m_CameraRotation);

            Hazel::Renderer::BeginScene(m_Camera);

            glm::mat4 transform = glm::translate(glm::mat4(1.0f), m_SquarePosition);
            glm::mat4 scale = glm::scale(glm::mat4(1.0f), glm::vec3(0.1f));

            for (int i = 0; i < 5; i++)
            {
                glm::vec3 pos(i * 0.11f, 0.0f, 0.0f);
                glm::mat4 transform = glm::translate(glm::mat4(1.0f), pos) * scale;
                Hazel::Renderer::Submit(squareShader, squareVertexArray, transform);
            }
            

            Hazel::Renderer::Submit(squareShader, squareVertexArray, transform);
            // Hazel::Renderer::Submit(trangleShader, vertexArray);

            Hazel::Renderer::EndScene();
        }

        void OnImGuiRender() override{
            

        }

        void OnEvent(Hazel::Event& event) override {

            
        }

    private:
        std::shared_ptr<Hazel::Shader> trangleShader;
        std::shared_ptr<Hazel::Shader> squareShader;
        std::shared_ptr<Hazel::VertexArray> vertexArray;
        std::shared_ptr<Hazel::VertexArray> squareVertexArray;

        Hazel::OrthographicCamera m_Camera;
        glm::vec3 m_CameraPosition;
        float m_CameraMoveSpeed = 2.f;

        float m_CameraRotation= 0.0f;
        float m_CameraRotationSpeed= 30.f;

        float m_SquareMoveSpeed = 1.f;

        glm::vec3 m_SquarePosition;
};


class Sandbox : public Hazel::Application {
    public:
        Sandbox() {
            PushLayer(new ExampleLayer());
        }

        ~Sandbox() {

        }
};

Hazel::Application* Hazel::CreateApplication() {
    return new Sandbox();
}
