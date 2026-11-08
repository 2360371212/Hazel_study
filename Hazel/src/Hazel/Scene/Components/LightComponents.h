#pragma once

#include "Hazel/Renderer/Lighting/LightTypes.h"

namespace Hazel
{
    struct DirectionalLightComponent
    {
        LightProperties Properties;
    };

    struct PointLightComponent
    {
        LightProperties Properties;

        float Range = 10.0f;
    };

    struct SpotLightComponent
    {
        LightProperties Properties;

        float Range = 10.0f;

        float InnerConeAngle = 20.0f;
        float OuterConeAngle = 30.0f;
    };
}