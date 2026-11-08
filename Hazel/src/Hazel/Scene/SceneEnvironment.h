#pragma once

#include <cstdint>
#include <glm/glm.hpp>
#include "Hazel/Core/Core.h"
#include "Hazel/Renderer/Common/Texture.h"

namespace Hazel
{
    struct SceneEnvironment
    {
        glm::vec4 ClearColor{
            0.035f,
            0.045f,
            0.065f,
            1.0f
        };

        // 常量环境补光。
        glm::vec3 AmbientColor{ 1.0f };
        float AmbientIntensity = 0.0f;

        // 当前项目使用二维全景环境贴图。
        Ref<Texture2D> EnvironmentMap;
        Ref<Texture2D> IrradianceMap;

        bool ShowBackground = true;
        bool EnvironmentLightingEnabled = false;

        float EnvironmentIntensity = 1.0f;
        float BackgroundIntensity = 1.0f;

        // 内部统一使用弧度，ImGui 可以用角度输入后转换。
        float RotationRadians = 0.0f;

        float Exposure = 1.0f;
    };
}