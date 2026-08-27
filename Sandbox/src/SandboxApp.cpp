#include "Hazel.h"

#include "imgui.h"


class ExampleLayer : public Hazel::Layer{
    public:
        ExampleLayer()
            : Layer("Example"){
            
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

                out vec3 v_Position;
                out vec4 v_Color;

                void main()
                {
                    v_Position = a_Position;
                    v_Color = a_Color;
                    gl_Position = vec4(a_Position, 1.0);
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
                -0.5f, -0.5f, 0.0f,
                0.5f, -0.5f, 0.0f,
                0.5f, 0.5f, 0.0f,
                -0.5f, 0.5f, 0.0f
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

                out vec3 v_Position;

                void main()
                {
                    v_Position = a_Position;
                    gl_Position = vec4(a_Position, 1.0);
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
        
        void OnUpdate() override{
            Hazel::RenderCommand::SetClearColor({0.1f, 0.1f, 0.1f, 1});
            Hazel::RenderCommand::Clear();

            Hazel::Renderer::BeginScene();

            squareShader->Bind();
            Hazel::Renderer::Submit(squareVertexArray);

            trangleShader->Bind();
            Hazel::Renderer::Submit(vertexArray);

            Hazel::Renderer::EndScene();
        }

        void OnImGuiRender() override{
            

        }

        void OnEvent(Hazel::Event& event) override {
            if (event.GetEventType() == Hazel::EventType::KeyPressed)
            {
                Hazel::KeyPressedEvent& e = dynamic_cast<Hazel::KeyPressedEvent&>(event);
                int code = e.GetKeyCode();
                HZ_TRACE("{0}", (char)code);
            }
        }
    private:
        std::unique_ptr<Hazel::Shader> trangleShader;
        std::unique_ptr<Hazel::Shader> squareShader;
        std::shared_ptr<Hazel::VertexArray> vertexArray;
        std::shared_ptr<Hazel::VertexArray> squareVertexArray;



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
