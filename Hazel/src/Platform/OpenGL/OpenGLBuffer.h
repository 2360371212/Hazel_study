#pragma once
#include "Hazel/Renderer/Buffer.h"


namespace Hazel{
    class OpenGLVertexBuffer : public VertexBuffer{
        public:
            OpenGLVertexBuffer(float* vertices, uint32_t size);
            virtual ~OpenGLVertexBuffer() override;
            virtual void Bind() const override;
            virtual void UnBind() const override;
        private:
            uint32_t m_RendererID;
    };

    class OpenGLIndexBuffer : public IndexBuffer{
        public:
            OpenGLIndexBuffer(uint32_t* indeces, uint32_t count);
            virtual ~OpenGLIndexBuffer() override;
            virtual void Bind() const override;
            virtual void UnBind() const override;

            inline virtual uint32_t GetCount() const {return m_Count;}

        private:
            uint32_t m_RendererID;
            uint32_t m_Count;
    };
}