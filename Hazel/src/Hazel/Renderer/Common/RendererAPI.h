#pragma once
#include <glm/glm.hpp>
#include "VertexArray.h"
#include "Hazel/Renderer/Common/RenderState.h"

namespace Hazel{
    class RendererAPI{
        public:
            enum class API{
                None = 0, OpenGL = 1
            };
        public:
            virtual void SetClearColor(const glm::vec4& color) = 0;
            virtual void Clear() = 0;
            virtual void DrawIndexed(const Ref<VertexArray>& vertexArray, uint32_t indexCount) = 0;
            virtual void SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height) = 0;
            // 是否由图形接口在写入 sRGB 颜色附件时执行编码。
            virtual void SetSRGBWrite(bool enabled) = 0;
            // 是否启用裁剪测试。
            virtual void SetScissorTest(bool enabled) = 0;
            // 控制颜色各通道是否允许写入。
            virtual void SetColorWrite(bool red, bool green, bool blue, bool alpha) = 0;
            // 是否允许写入深度。
            virtual void SetDepthWrite(bool enabled) = 0;
            // 设置深度清除值。
            virtual void SetClearDepth(float depth) = 0;

            virtual void SetRenderState(const RenderState& state) = 0;

            inline static API GetAPI() {return s_API;};

            virtual void Init() = 0;

        private:
            static API s_API;
    };
}