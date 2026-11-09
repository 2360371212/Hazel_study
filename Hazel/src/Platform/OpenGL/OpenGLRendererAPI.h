#pragma once

#include "Hazel/Renderer/Common/RendererAPI.h"

namespace Hazel{
    class OpenGLRendererAPI : public RendererAPI{
        public:
            virtual void SetClearColor(const glm::vec4& color) override;
            virtual void Clear() override;
            virtual void DrawIndexed(const Ref<VertexArray>& vertexArray, uint32_t indexCount = 0) override; 
            virtual void Init() override;
            virtual void SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height) override;
            virtual void SetSRGBWrite(bool enabled) override;
            virtual void SetScissorTest(bool enabled) override;
            virtual void SetColorWrite(bool red, bool green, bool blue, bool alpha) override;
            virtual void SetDepthWrite(bool enabled) override;
            virtual void SetClearDepth(float depth) override;
            virtual void SetRenderState(const RenderState& state) override;
    };
}