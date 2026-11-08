#pragma once

#include "Hazel/Core/Core.h"
#include "Hazel/Renderer/Common/Texture.h"

namespace Hazel
{
    struct IBLResources
    {
        // 原始环境转换得到的 Cubemap。
        Ref<TextureCube> EnvironmentMap;

        // 漫反射环境光。
        Ref<TextureCube> IrradianceMap;

        // 镜面环境光：
        // 不同 mip 保存不同粗糙度的过滤结果。
        Ref<TextureCube> PrefilteredMap;

        // 镜面 BRDF 积分查找表。
        Ref<Texture2D> BRDFLUT;
    };
}