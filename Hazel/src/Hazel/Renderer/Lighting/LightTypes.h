#pragma once
#include "Hazel/Core/Core.h"
#include <glm/glm.hpp>

namespace Hazel
{
    struct LightProperties
    {
        glm::vec3 Color{ 1.0f };
        float Intensity = 1.0f;
        bool Enabled = true;
    };

    // 方向光
    struct DirectionalLight
    {
        LightProperties Properties;

        // 世界空间，表示光线传播方向。
        glm::vec3 Direction{ -0.4f, -0.8f, -0.5f };
    };

    // 点光源
    struct PointLight
    {
        LightProperties Properties;

        glm::vec3 Position{ 0.0f };

        // 影响范围，由具体衰减公式实现平滑截止。
        float Range = 10.0f;
    };

    // 聚光灯
    struct SpotLight
    {
        LightProperties Properties;

        glm::vec3 Position{ 0.0f };
        glm::vec3 Direction{ 0.0f, 0.0f, -1.0f };

        float Range = 10.0f;

        // 输入端保证：0 <= InnerConeAngle <= OuterConeAngle < 90。
        float InnerConeAngle = 0.f;
        float OuterConeAngle = 45.f;
    };
}