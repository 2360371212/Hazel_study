#pragma once

#include "Hazel/Core/Core.h"

#include "Hazel/Renderer/RenderView.h"
#include "Hazel/Renderer/Scene/RenderSceneData.h"
#include "Hazel/Renderer/Pipeline/RenderFrameResources.h"

namespace Hazel
{
    struct RenderPassContext
    {
        // 渲染的场景文件包裹，网格，灯光，环境
        const RenderSceneData& Scene;
        // 渲染的视口，包括变换矩阵等信息
        const RenderView& View;
        // Frame资源
        const RenderFrameResources& Resources;
    };

    class HAZEL_API RenderPass
    {
    public:
        virtual ~RenderPass() = default;
        virtual const char* GetName() const = 0;
        virtual void Execute(const RenderPassContext& context) = 0;
    };
}