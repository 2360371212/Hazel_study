#pragma once

#include "Hazel/Core/Core.h"
#include "Hazel/Renderer/Common/Framebuffer.h"
#include "Hazel/Renderer/Common/Texture.h"
#include "Hazel/Renderer/Scene/RenderSceneData.h"
#include "Hazel/Renderer/RenderView.h"

namespace Hazel
{
    class HAZEL_API RenderPipeline
    {
    public:
        virtual ~RenderPipeline() = default;

        // output 为空时，输出到窗口默认 Framebuffer。
        virtual void Render(const RenderSceneData& scene, const RenderView& view, const Ref<Framebuffer>& output = nullptr) = 0;

        // 获取色调映射之前的线性 HDR 颜色。
        // 第一次有效 Render 调用之前返回 nullptr。
        virtual Ref<Texture2D> GetHDRColor() const = 0;
    };
}