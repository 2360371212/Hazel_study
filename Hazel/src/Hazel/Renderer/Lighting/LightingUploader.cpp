#include "hzpch.h"
#include "LightingUploader.h"
#include "SceneLighting.h"
#include "Hazel/Renderer/Common/Shader.h"

namespace Hazel
{
    namespace
    {
        constexpr float DirectionLengthSquaredEpsilon = 1e-12f;

        bool IsFinite(const glm::vec3& value) {
            return std::isfinite(value.x) && std::isfinite(value.y) && std::isfinite(value.z);
        }

        bool IsValidColor(const glm::vec3& color) {
            return IsFinite(color) && color.x >= 0.0f && color.y >= 0.0f && color.z >= 0.0f;
        }

        bool IsValidIntensity(float intensity) {
            return std::isfinite(intensity) && intensity >= 0.0f;
        }

        bool IsValidProperties(const LightProperties& properties) {
            return IsValidColor(properties.Color) && IsValidIntensity(properties.Intensity);
        }

        bool IsValidRange(float range) {
            return std::isfinite(range) && range > 0.0f;
        }


        // 方向归一化
        bool TryNormalizeDirection(const glm::vec3& direction, glm::vec3& normalized){
            const float lengthSquared = glm::dot(direction, direction);

            if (!std::isfinite(lengthSquared) || lengthSquared <= DirectionLengthSquaredEpsilon){
                return false;
            }
            normalized = direction / std::sqrt(lengthSquared);
            return true;
        }

        // 传入角度,判断是否符合要求
        bool IsValidSpotAngles(float innerAngle, float outerAngle)
        {
            return innerAngle <= outerAngle && outerAngle < 90.f;
        }

        void UploadFloat3(Shader& shader, const std::string& name, glm::vec3 value)
        {
            shader.SetFloat3(name, value);
        }

        void UploadProperties(Shader& shader, const std::string& prefix, const LightProperties& properties)
        {
            // 上传光源颜色
            UploadFloat3(shader, prefix + ".Color", properties.Color);
            shader.SetFloat(prefix + ".Intensity", properties.Intensity);
        }

        void UploadAmbient(Shader& shader, const AmbientLight& ambient , LightingUploadStats& stats)
        {
            if (!IsValidColor(ambient.Color) || !IsValidIntensity(ambient.Intensity))
            {
                stats.InvalidAmbient = true;

                // 无效时明确上传零值，
                // 避免继续使用上一次调用留下的数据。
                UploadFloat3(shader, "u_AmbientColor", glm::vec3(0.0f));
                shader.SetFloat("u_AmbientIntensity", 0.0f);
                return;
            }
            UploadFloat3(shader, "u_AmbientColor", ambient.Color);
            shader.SetFloat("u_AmbientIntensity", ambient.Intensity);
        }

        void UploadDirectionalLights(Shader& shader, const SceneLighting& lighting, LightingUploadStats& stats) {
            for (const auto& light : lighting.DirectionalLights)
            {
                if (!light.Properties.Enabled)
                    continue;

                glm::vec3 direction{ 0.0f };

                if (!IsValidProperties(light.Properties) || !TryNormalizeDirection(light.Direction, direction)) {
                    ++stats.InvalidLightCount;
                    continue;
                }

                if (stats.DirectionalCount >= LightingUploader::MaxDirectionalLights) {
                    ++stats.OverflowLightCount;
                    continue;
                }

                const std::string prefix = "u_DirectionalLights[" + std::to_string(stats.DirectionalCount) + "]";

                UploadProperties(shader, prefix, light.Properties);
                UploadFloat3(shader, prefix + ".Direction", direction);
                ++stats.DirectionalCount;
            }

            shader.SetInt("u_DirectionalLightCount", static_cast<int>(stats.DirectionalCount));
        }

        void UploadPointLights(Shader& shader, const SceneLighting& lighting, LightingUploadStats& stats) {
            for (const auto& light : lighting.PointLights)
            {
                if (!light.Properties.Enabled)
                    continue;

                if (!IsValidProperties(light.Properties) || !IsFinite(light.Position) || !IsValidRange(light.Range)) {
                    ++stats.InvalidLightCount;
                    continue;
                }

                if (stats.PointCount >= LightingUploader::MaxPointLights) {
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

                glm::vec3 direction{ 0.0f };

                if (!IsValidProperties(light.Properties) || !IsFinite(light.Position) || !IsValidRange(light.Range) || !TryNormalizeDirection(light.Direction, direction) ||
                    !IsValidSpotAngles(light.InnerConeAngle, light.OuterConeAngle)) {
                    ++stats.InvalidLightCount;
                    continue;
                }

                if (stats.SpotCount >= LightingUploader::MaxSpotLights) {
                    ++stats.OverflowLightCount;
                    continue;
                }

                const std::string prefix = "u_SpotLights[" + std::to_string(stats.SpotCount) + "]";

                UploadProperties(shader, prefix, light.Properties);
                UploadFloat3(shader, prefix + ".Position", light.Position);
                UploadFloat3(shader, prefix + ".Direction", direction);
                shader.SetFloat(prefix + ".Range", light.Range);

                // CPU 保存角度，Shader 使用余弦值。
                shader.SetFloat(prefix + ".InnerCos", std::cos(light.InnerConeAngle));
                shader.SetFloat(prefix + ".OuterCos", std::cos(light.OuterConeAngle));
                ++stats.SpotCount;
            }
            
            shader.SetInt("u_SpotLightCount", static_cast<int>(stats.SpotCount));
        }
    }

    // 上传场景中的所有光源
    LightingUploadStats LightingUploader::Upload(Shader& shader, const SceneLighting& lighting)
    {
        LightingUploadStats stats;

        UploadAmbient(shader, lighting.Ambient, stats);
        UploadDirectionalLights(shader, lighting, stats);
        UploadPointLights(shader, lighting, stats);
        UploadSpotLights(shader, lighting, stats);

        return stats;
    }
}