#pragma once
#include "RendererAPI.h"
#include "VertexArray.h"

namespace Hazel{

    
    class RenderCommand{
        public:
            inline static void DrawIndexed(const Ref<VertexArray>& vertexArray, uint32_t count = 0)
		    {
			    s_RendererAPI->DrawIndexed(vertexArray, count);
		    }

            inline static void SetClearColor(const glm::vec4& color) {
                s_RendererAPI->SetClearColor(color);
            }

            inline static void Clear() {
                s_RendererAPI->Clear();
            }                  

            inline static void Init(){
                s_RendererAPI->Init();
            }

            inline static void SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height){
                s_RendererAPI->SetViewport(x, y, width, height);
            }

            inline static void SetSRGBWrite(bool enabled) {
                s_RendererAPI->SetSRGBWrite(enabled);
            }

            inline static void SetScissorTest(bool enabled) {
                s_RendererAPI->SetScissorTest(enabled);
            }

            inline static void SetColorWrite(bool red, bool green, bool blue, bool alpha) {
                s_RendererAPI->SetColorWrite(red, green, blue, alpha);
            }

            inline static void SetDepthWrite(bool enabled) {
                s_RendererAPI->SetDepthWrite(enabled);
            }

            inline static void SetClearDepth(float depth) {
                s_RendererAPI->SetClearDepth(depth);
            }

            inline static void SetRenderState(const RenderState& state){
                s_RendererAPI->SetRenderState(state);
            }

        private:
            static RendererAPI* s_RendererAPI;
    };
}