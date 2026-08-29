#include "Hazel.h"

#include "imgui.h"
#include "glm/gtc/matrix_transform.hpp"
#include "Platform/OpenGL/OpenGLShader.h"
#include "glm/gtc/type_ptr.hpp"

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

            trangleShader.reset(Hazel::Shader::Create(vertexSrc, fragmentSrc));

            squareVertexArray.reset(Hazel::VertexArray::Create());

            float squareVertices[4 * 5] = {
                -0.5f, -0.5f, 0.0f, 0.0f, 0.0f,
                0.5f, -0.5f, 0.0f, 1.0f, 0.0f,
                0.5f, 0.5f, 0.0f, 1.0f, 1.0f,
                -0.5f, 0.5f, 0.0f, 0.0f, 1.0f
            };

            
            std::shared_ptr<Hazel::VertexBuffer> squareVertexBuffer;
            squareVertexBuffer.reset(Hazel::VertexBuffer::Create(squareVertices, sizeof(squareVertices)));
            Hazel::BufferLayout squareLayout= {
                {Hazel::ShaderDataType::Float3, "a_Position"},
                {Hazel::ShaderDataType::Float2, "a_TexCoord"}
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

            std::string flatColorShaderFragmentSrc = R"(
                #version 330 core      

                layout(location = 0) out vec4 color;

                in vec3 v_Position;

                uniform vec3 u_Color;

                void main()
                {
                    color = vec4(u_Color, 1);
                }
            )";
            squareShader.reset(Hazel::Shader::Create(vertexSrc2, flatColorShaderFragmentSrc));

            std::string textureShaderVertexSrc= R"(
                #version 330 core      

                layout(location = 0) in vec3 a_Position;
                layout(location = 1) in vec2 a_TexCoord;

                uniform mat4 u_ViewProjection;
                uniform mat4 u_Transform;

                out vec2 v_TexCoord;

                void main()
                {
                    v_TexCoord = a_TexCoord;
                    gl_Position = u_ViewProjection * u_Transform * vec4(a_Position, 1.0);
                }
            )";

            std::string textureShaderFragmentSrc= R"(
                #version 330 core      

                layout(location = 0) out vec4 color;

                in vec2 v_TexCoord;

                uniform sampler2D u_Texture;

                void main()
                {
                    color = texture(u_Texture, v_TexCoord);
                }
            )";
            m_TextureShader.reset(Hazel::Shader::Create(textureShaderVertexSrc, textureShaderFragmentSrc));

            m_Texture = Hazel::Texture2D::Create("D:/Project/cpp/Hazel/Sandbox/assets/texture/test.jpg");

            std::dynamic_pointer_cast<Hazel::OpenGLShader>(m_TextureShader)->Bind();
            // select the texture slot
            std::dynamic_pointer_cast<Hazel::OpenGLShader>(m_TextureShader)->UploadUniformInt("u_Texture", 0);
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

            // if (Hazel::Input::IsKeyPressed(HZ_KEY_J)){
            //     m_SquarePosition.x -= m_SquareMoveSpeed * deltaTime;
            // } else if (Hazel::Input::IsKeyPressed(HZ_KEY_L)){
            //     m_SquarePosition.x += m_SquareMoveSpeed * deltaTime;
            // }
            // if (Hazel::Input::IsKeyPressed(HZ_KEY_I)){
            //     m_SquarePosition.y += m_SquareMoveSpeed * deltaTime;
            // } else if (Hazel::Input::IsKeyPressed(HZ_KEY_K)){
            //     m_SquarePosition.y -= m_SquareMoveSpeed * deltaTime;
            // }


            Hazel::RenderCommand::SetClearColor({0.1f, 0.1f, 0.1f, 1});
            Hazel::RenderCommand::Clear();
            m_Camera.SetPosition(m_CameraPosition);
            m_Camera.SetRotation(m_CameraRotation);

            Hazel::Renderer::BeginScene(m_Camera);

            glm::mat4 transform = glm::translate(glm::mat4(1.0f), m_SquarePosition);
            glm::mat4 scale = glm::scale(glm::mat4(1.0f), glm::vec3(0.1f));

            std::dynamic_pointer_cast<Hazel::OpenGLShader>(squareShader)->Bind();
            std::dynamic_pointer_cast<Hazel::OpenGLShader>(squareShader)->UploadUniformFloat3("u_Color", m_SquareColor);

            for(int y = 0 ; y < 20; y++){
                for(int x = 0; x < 20 ; x++){
                    glm::vec3 pos(x * 0.11f, y * 0.11f, 0.0f);
                    glm::mat4 transform = glm::translate(glm::mat4(1.0f),pos) * scale;
                    Hazel::Renderer::Submit(squareShader, squareVertexArray, transform);
                }
            }

            m_Texture->Bind();
            Hazel::Renderer::Submit(m_TextureShader, squareVertexArray, glm::scale(glm::mat4(1.0f), glm::vec3(1.5f)));

            // Hazel::Renderer::Submit(trangleShader, vertexArray);
            Hazel::Renderer::EndScene();
        }

        void OnImGuiRender() override{
            ImGui::Begin("Settings");
            ImGui::ColorEdit3("Square Color", glm::value_ptr(m_SquareColor));
            ImGui::End();
        }

        void OnEvent(Hazel::Event& event) override {

            
        }

    private:
        Hazel::Ref<Hazel::Shader> trangleShader;
        Hazel::Ref<Hazel::Shader> squareShader, m_TextureShader;
        Hazel::Ref<Hazel::VertexArray> vertexArray;
        Hazel::Ref<Hazel::VertexArray> squareVertexArray;

        Hazel::Ref<Hazel::Texture2D> m_Texture;

        Hazel::OrthographicCamera m_Camera;
        glm::vec3 m_CameraPosition;
        float m_CameraMoveSpeed = 2.f;

        float m_CameraRotation= 0.0f;
        float m_CameraRotationSpeed= 30.f;

        float m_SquareMoveSpeed = 1.f;

        glm::vec3 m_SquarePosition;

        glm::vec3 m_SquareColor = {0.2f, 0.3f, 0.8f};
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
