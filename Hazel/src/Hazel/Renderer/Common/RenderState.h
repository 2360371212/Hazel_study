#pragma once

namespace Hazel
{
    // 描述各种渲染模式
    // 混合模式
    enum class BlendMode {
        Opaque,
        AlphaBlend,
        Additive
    };

    // 是否剔除背面
    enum class CullMode {
        None,
        Front,
        Back
    };

    // 渲染状态
    struct RenderState {
        BlendMode Blend = BlendMode::Opaque;
        CullMode Cull = CullMode::Back;

        bool DepthTest = true;
        bool DepthWrite = true;
    };
}