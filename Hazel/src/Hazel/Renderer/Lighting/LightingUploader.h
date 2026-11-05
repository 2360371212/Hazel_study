#pragma once
#include "Hazel/Core/Core.h"
#include "Hazel/Renderer/Lighting/SceneLighting.h"
#include "Hazel/Renderer/Common/Shader.h"

namespace Hazel
{
    struct LightingUploadStats {
        uint32_t DirectionalCount = 0;
        uint32_t PointCount = 0;
        uint32_t SpotCount = 0;

        uint32_t InvalidLightCount = 0;
        uint32_t OverflowLightCount = 0;

        bool InvalidAmbient = false;
    };

    class LightingUploader {
    public:
        // 必须与使用这套接口的 GLSL 数组容量一致。
        static constexpr uint32_t MaxDirectionalLights = 4;
        static constexpr uint32_t MaxPointLights = 8;
        static constexpr uint32_t MaxSpotLights = 8;

        static LightingUploadStats Upload(Shader& shader, const SceneLighting& lighting);
    };
}