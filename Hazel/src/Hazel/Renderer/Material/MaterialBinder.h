#pragma once

#include <cstdint>
#include "MaterialInstance.h"

namespace Hazel
{
    class MaterialBinder
    {
    public:
        // 绑定 Shader，上传完整材质参数，绑定材质纹理。
        static uint32_t Bind(const MaterialInstance& instance, uint32_t firstTextureSlot = 0, uint32_t textureSlotLimit = 32);
    };
}