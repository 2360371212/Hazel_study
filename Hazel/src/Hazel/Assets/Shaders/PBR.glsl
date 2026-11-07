#type vertex
#version 330 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec3 a_Normal;
layout(location = 2) in vec2 a_TexCoord;

uniform mat4 u_ViewProjection;
uniform mat4 u_Model;
uniform mat3 u_NormalMatrix;

out vec3 v_WorldPosition;
out vec3 v_WorldNormal;
out vec2 v_TexCoord;

void main()
{
    vec4 worldPosition =
        u_Model * vec4(a_Position, 1.0);

    v_WorldPosition = worldPosition.xyz;
    v_WorldNormal = u_NormalMatrix * a_Normal;
    v_TexCoord = a_TexCoord;

    gl_Position =
        u_ViewProjection * worldPosition;
}

#type fragment
#version 330 core

layout(location = 0) out vec4 o_Color;

in vec3 v_WorldPosition;
in vec3 v_WorldNormal;
in vec2 v_TexCoord;

const float PI = 3.14159265359;

const int MAX_DIRECTIONAL_LIGHTS = 4;
const int MAX_POINT_LIGHTS = 8;
const int MAX_SPOT_LIGHTS = 8;

struct Material
{
    vec4 BaseColor;
    float Metallic;
    float Roughness;
};

struct DirectionalLight
{
    vec3 Color;
    float Intensity;

    // 光线传播方向。
    vec3 Direction;
};

struct PointLight
{
    vec3 Color;
    float Intensity;

    vec3 Position;
    float Range;
};

struct SpotLight
{
    vec3 Color;
    float Intensity;

    vec3 Position;
    float Range;

    vec3 Direction;

    float InnerCos;
    float OuterCos;
};

uniform Material u_Material;
uniform sampler2D u_BaseColorMap;

uniform vec3 u_CameraPosition;
uniform float u_Exposure;

uniform vec3 u_AmbientColor;
uniform float u_AmbientIntensity;

uniform int u_DirectionalLightCount;
uniform int u_PointLightCount;
uniform int u_SpotLightCount;

uniform DirectionalLight
    u_DirectionalLights[MAX_DIRECTIONAL_LIGHTS];

uniform PointLight
    u_PointLights[MAX_POINT_LIGHTS];

uniform SpotLight
    u_SpotLights[MAX_SPOT_LIGHTS];

// --------------------------------------------------
// 基础工具
// --------------------------------------------------

vec3 SafeNormalize(vec3 value)
{
    return value *
        inversesqrt(max(dot(value, value), 1e-12));
}

vec3 SRGBToLinear(vec3 color)
{
    vec3 low = color / 12.92;

    vec3 high = pow(
        (color + 0.055) / 1.055,
        vec3(2.4));

    return mix(
        high,
        low,
        lessThanEqual(color, vec3(0.04045)));
}

vec3 LinearToSRGB(vec3 color)
{
    vec3 low = color * 12.92;

    vec3 high =
        1.055 * pow(color, vec3(1.0 / 2.4)) -
        0.055;

    return mix(
        high,
        low,
        lessThanEqual(color, vec3(0.0031308)));
}

// 沿用现有灯光的范围衰减约定。
float DistanceAttenuation(
    float distanceSquared,
    float range)
{
    float normalizedDistanceSquared =
        distanceSquared / (range * range);

    float fade = max(
        1.0 -
        normalizedDistanceSquared *
        normalizedDistanceSquared,
        0.0);

    return (fade * fade) /
        max(distanceSquared, 0.01);
}

float SpotAttenuation(
    float cosTheta,
    float innerCos,
    float outerCos)
{
    // 相同内外锥角表示硬边缘。
    if (innerCos == outerCos)
        return step(outerCos, cosTheta);

    return smoothstep(
        outerCos,
        innerCos,
        cosTheta);
}

// --------------------------------------------------
// PBR BRDF
// --------------------------------------------------

float DistributionGGX(
    float nDotH,
    float roughness)
{
    float alpha = roughness * roughness;
    float alphaSquared = alpha * alpha;

    float denominator =
        nDotH * nDotH *
        (alphaSquared - 1.0) +
        1.0;

    return alphaSquared /
        (PI * denominator * denominator);
}

// 单方向 Smith GGX 遮蔽项。
float GeometryGGX(
    float nDotX,
    float alphaSquared)
{
    return (2.0 * nDotX) /
        (
            nDotX +
            sqrt(
                alphaSquared +
                (1.0 - alphaSquared) *
                nDotX * nDotX)
        );
}

float GeometrySmith(
    float nDotV,
    float nDotL,
    float roughness)
{
    float alpha = roughness * roughness;
    float alphaSquared = alpha * alpha;

    return
        GeometryGGX(nDotV, alphaSquared) *
        GeometryGGX(nDotL, alphaSquared);
}

vec3 FresnelSchlick(
    float hDotV,
    vec3 F0)
{
    float factor =
        pow(1.0 - hDotV, 5.0);

    return F0 + (1.0 - F0) * factor;
}

// N：表面法线。
// V：表面指向相机。
// L：表面指向光源。
// incidentLight：已包含颜色、强度和衰减的入射光。
vec3 EvaluateDirectBRDF(
    vec3 baseColor,
    float metallic,
    float roughness,
    vec3 N,
    vec3 V,
    vec3 L,
    vec3 incidentLight)
{
    float nDotV = max(dot(N, V), 0.0);
    float nDotL = max(dot(N, L), 0.0);

    if (nDotV <= 0.0 || nDotL <= 0.0)
        return vec3(0.0);

    vec3 H = SafeNormalize(V + L);

    float nDotH = clamp(dot(N, H), 0.0, 1.0);
    float hDotV = clamp(dot(H, V), 0.0, 1.0);

    vec3 F0 = mix(
        vec3(0.04),
        baseColor,
        metallic);

    float D = DistributionGGX(
        nDotH,
        roughness);

    float G = GeometrySmith(
        nDotV,
        nDotL,
        roughness);

    vec3 F = FresnelSchlick(
        hDotV,
        F0);

    vec3 specular =
        (D * G * F) /
        max(4.0 * nDotV * nDotL, 1e-6);

    vec3 kD =
        (vec3(1.0) - F) *
        (1.0 - metallic);

    vec3 diffuse =
        kD * baseColor / PI;

    return
        (diffuse + specular) *
        incidentLight *
        nDotL;
}

// --------------------------------------------------
// 主函数
// --------------------------------------------------

void main()
{
    vec3 textureColor = SRGBToLinear(
        texture(u_BaseColorMap, v_TexCoord).rgb);

    // BaseColor 参数本身按线性值使用。
    vec3 baseColor =
        u_Material.BaseColor.rgb *
        textureColor;

    float metallic = u_Material.Metallic;

    // 避免零粗糙度使 GGX 退化为无法直接采样的理想镜面。
    float roughness =
        max(u_Material.Roughness, 0.045);

    vec3 N = SafeNormalize(v_WorldNormal);

    vec3 V = SafeNormalize(
        u_CameraPosition - v_WorldPosition);

    vec3 result = vec3(0.0);

    // 方向光。
    for (int i = 0;
         i < u_DirectionalLightCount;
         ++i)
    {
        DirectionalLight light =
            u_DirectionalLights[i];

        vec3 L = -light.Direction;

        vec3 incidentLight =
            light.Color * light.Intensity;

        result += EvaluateDirectBRDF(
            baseColor,
            metallic,
            roughness,
            N,
            V,
            L,
            incidentLight);
    }

    // 点光源。
    for (int i = 0;
         i < u_PointLightCount;
         ++i)
    {
        PointLight light = u_PointLights[i];

        vec3 toLight =
            light.Position - v_WorldPosition;

        float distanceSquared =
            dot(toLight, toLight);

        vec3 L = SafeNormalize(toLight);

        float attenuation =
            DistanceAttenuation(
                distanceSquared,
                light.Range);

        vec3 incidentLight =
            light.Color *
            light.Intensity *
            attenuation;

        result += EvaluateDirectBRDF(
            baseColor,
            metallic,
            roughness,
            N,
            V,
            L,
            incidentLight);
    }

    // 聚光灯。
    for (int i = 0;
         i < u_SpotLightCount;
         ++i)
    {
        SpotLight light = u_SpotLights[i];

        vec3 toLight =
            light.Position - v_WorldPosition;

        float distanceSquared =
            dot(toLight, toLight);

        vec3 L = SafeNormalize(toLight);

        float cosTheta =
            dot(-L, light.Direction);

        float attenuation =
            DistanceAttenuation(
                distanceSquared,
                light.Range) *
            SpotAttenuation(
                cosTheta,
                light.InnerCos,
                light.OuterCos);

        vec3 incidentLight =
            light.Color *
            light.Intensity *
            attenuation;

        result += EvaluateDirectBRDF(
            baseColor,
            metallic,
            roughness,
            N,
            V,
            L,
            incidentLight);
    }

    // 调试用常量漫反射补光。
    // 默认设为零；后续用 IBL 替换。
    result +=
        baseColor *
        (1.0 - metallic) *
        u_AmbientColor *
        u_AmbientIntensity;

    // 第一阶段暂时在这里完成显示输出。
    vec3 mapped =
        vec3(1.0) -
        exp(-result * u_Exposure);

    o_Color = vec4(
        LinearToSRGB(mapped),
        1.0);
}