#pragma once

#include <cstdint>
#include <string>

#include "Hazel/Renderer/Common/Texture.h"

namespace Hazel
{
    class HAZEL_API EnvironmentMapLoader
    {
    public:
        // 初始化时在 CPU 上将等距柱状 HDR 转为六个面，上传线性浮点 Cubemap。
        static Ref<TextureCube> LoadHDR(const std::string& path, uint32_t faceSize = 512);
    };
}
