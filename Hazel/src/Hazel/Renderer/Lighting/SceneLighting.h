#pragma once
#include "Hazel/Core/Core.h"
#include "LightTypes.h"

namespace Hazel
{
    // 环境光
    struct AmbientLight
    {
        glm::vec3 Color{ 1.0f };
        float Intensity = 0.1f;
    };

    struct SceneLighting
    {
        AmbientLight Ambient;

        std::vector<DirectionalLight> DirectionalLights;
        std::vector<PointLight> PointLights;
        std::vector<SpotLight> SpotLights;
    };
}