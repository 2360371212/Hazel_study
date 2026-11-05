#pragma once
#pragma once

#include <cstdint>

namespace Hazel
{
    enum class AssetType : uint8_t
    {
        None = 0,

        Texture2D,
        Shader,
        Mesh,
        Model,
        Material,
        Scene
    };
}