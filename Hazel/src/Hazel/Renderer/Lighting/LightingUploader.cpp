#include "hzpch.h"
#include "LightingUploader.h"
#include "Hazel/Core/Core.h"
#include "Hazel/Core/Log.h"

#include <cassert>
#include <cmath>
#include <string>

#include "SceneLighting.h"
#include "Hazel/Renderer/Common/Shader.h"

namespace Hazel
{
    namespace
    {
        void UploadFloat3(Shader& shader, const std::string& name, glm::vec3 value) {
            shader.SetFloat3(name, value);
        }

        glm::vec3 NormalizeDirection(const glm::vec3& direction) {
            HZ_CORE_ASSERT(glm::dot(direction, direction) > 0.0f, "Light direction must not be zero");
            return glm::normalize(direction);
        }

        void UploadProperties(Shader& shader, const std::string& prefix, const LightProperties& properties) {
            UploadFloat3(shader, prefix + ".Color", properties.Color);
            shader.SetFloat(prefix + ".Intensity", properties.Intensity);
        }

        void UploadAmbient(Shader& shader, const AmbientLight& ambient) {
            UploadFloat3(shader, "u_AmbientColor", ambient.Color);
            shader.SetFloat("u_AmbientIntensity", ambient.Intensity);
        }

        void UploadDirectionalLights(Shader& shader, const SceneLighting& lighting, LightingUploadStats& stats) {
            for (const auto& light : lighting.DirectionalLights)
            {
                if (!light.Properties.Enabled)
                    continue;

                if (stats.DirectionalCount >= LightingUploader::MaxDirectionalLights) {
                    ++stats.OverflowLightCount;
                    continue;
                }
                const std::string prefix = "u_DirectionalLights[" + std::to_string(stats.DirectionalCount) + "]";
                UploadProperties(shader, prefix, light.Properties);
                UploadFloat3(shader, prefix + ".Direction", NormalizeDirection(light.Direction));
                ++stats.DirectionalCount;
            }

            shader.SetInt("u_DirectionalLightCount", static_cast<int>(stats.DirectionalCount));
        }

        void UploadPointLights(Shader& shader, const SceneLighting& lighting, LightingUploadStats& stats) {
            for (const auto& light : lighting.PointLights) {
                if (!light.Properties.Enabled)
                    continue;

                if (stats.PointCount >= LightingUploader::MaxPointLights)
                {
                    ++stats.OverflowLightCount;
                    continue;
                }

                const std::string prefix = "u_PointLights[" + std::to_string(stats.PointCount) + "]";

                UploadProperties(shader, prefix, light.Properties);
                UploadFloat3(shader, prefix + ".Position", light.Position);

                shader.SetFloat(prefix + ".Range", light.Range);
                ++stats.PointCount;
            }

            shader.SetInt("u_PointLightCount", static_cast<int>(stats.PointCount));
        }

        void UploadSpotLights(Shader& shader, const SceneLighting& lighting, LightingUploadStats& stats)
        {
            for (const auto& light : lighting.SpotLights) {
                if (!light.Properties.Enabled)
                    continue;

                if (stats.SpotCount >= LightingUploader::MaxSpotLights) {
                    ++stats.OverflowLightCount;
                    continue;
                }

                const std::string prefix = "u_SpotLights[" + std::to_string(stats.SpotCount) + "]";

                UploadProperties(shader, prefix, light.Properties);
                UploadFloat3(shader, prefix + ".Position", light.Position);

                UploadFloat3(shader, prefix + ".Direction", NormalizeDirection(light.Direction));

                shader.SetFloat(prefix + ".Range", light.Range);

                // CPU 保存度数，GLSL 接收余弦值。
                const float innerCos = std::cos(glm::radians(light.InnerConeAngle));

                const float outerCos = std::cos(glm::radians(light.OuterConeAngle));

                shader.SetFloat(prefix + ".InnerCos", innerCos);
                shader.SetFloat(prefix + ".OuterCos", outerCos);
                ++stats.SpotCount;
            }

            shader.SetInt("u_SpotLightCount", static_cast<int>(stats.SpotCount));
        }
    }

    LightingUploadStats LightingUploader::Upload(Shader& shader, const SceneLighting& lighting) {
        LightingUploadStats stats;
        UploadAmbient(shader, lighting.Ambient);
        UploadDirectionalLights(shader, lighting, stats);
        UploadPointLights(shader, lighting, stats);
        UploadSpotLights(shader, lighting, stats);

        return stats;
    }
}