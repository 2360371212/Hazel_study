#pragma once

#include "Hazel/Core/Core.h"
#include "Hazel/Renderer/Common/Framebuffer.h"

namespace Hazel
{
    struct RenderFrameResources
    {
        // Color：RGBA16F
        // Depth：Depth32F
        Ref<Framebuffer> SceneHDR;
    };
}