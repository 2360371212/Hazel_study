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

// 与 LightingUploader 中的容量保持一致。
const int MAX_DIRECTIONAL_LIGHTS = 4;
const int MAX_POINT_LIGHTS = 8;
const int MAX_SPOT_LIGHTS = 8;

struct Material
{
    vec4 BaseColor;

    float SpecularStrength;
    float Shininess;
};

struct DirectionalLight
{
    vec3 Color;
    float Intensity;

    // 光线传播方向，CPU 上传前已经归一化。
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

uniform sampler2D u_EnvironmentMap;
uniform sampler2D u_IrradianceMap;
uniform int u_EnvironmentEnabled;
uniform float u_EnvironmentIntensity;
uniform float u_ReflectionStrength;
uniform float u_EnvironmentRotation;

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

// 对相机、光源和片元位置重合等情况进行数值处理。
vec3 SafeNormalize(vec3 value)
{
    return value *
        inversesqrt(max(dot(value, value), 1e-12));
}

vec3 EnvironmentTexel(sampler2D image, ivec2 position)
{
    ivec2 size = textureSize(image, 0);
    position.x = (position.x % size.x + size.x) % size.x;
    position.y = clamp(position.y, 0, size.y - 1);
    return texelFetch(image, position, 0).rgb;
}

vec3 SampleEnvironment(sampler2D image, vec3 direction)
{
    float c = cos(u_EnvironmentRotation);
    float s = sin(u_EnvironmentRotation);
    direction = vec3(
        c * direction.x + s * direction.z,
        direction.y,
        -s * direction.x + c * direction.z);

    const float PI = 3.14159265359;
    vec2 uv = vec2(
        atan(direction.z, direction.x) / (2.0 * PI) + 0.5,
        asin(clamp(direction.y, -1.0, 1.0)) / PI + 0.5);

    // Bilinear filtering with horizontal wrap and vertical clamp.
    // The existing Texture2D uses nearest magnification and repeat on both axes.
    vec2 pixel = uv * vec2(textureSize(image, 0)) - 0.5;
    ivec2 base = ivec2(floor(pixel));
    vec2 weight = fract(pixel);

    vec3 bottom = mix(
        EnvironmentTexel(image, base),
        EnvironmentTexel(image, base + ivec2(1, 0)), weight.x);
    vec3 top = mix(
        EnvironmentTexel(image, base + ivec2(0, 1)),
        EnvironmentTexel(image, base + ivec2(1, 1)), weight.x);

    return mix(bottom, top, weight.y);
}

// 带平滑范围截止的距离衰减。
// 当前将 Intensity 作为此模型下的强度系数。
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

// L：表面指向光源。
// V：表面指向相机。
vec3 EvaluateBlinnPhong(
    vec3 baseColor,
    vec3 N,
    vec3 V,
    vec3 L,
    vec3 incidentLight)
{
    float nDotL = max(dot(N, L), 0.0);

    // 背光面不接受这盏灯的直接光。
    if (nDotL <= 0.0)
        return vec3(0.0);

    vec3 H = SafeNormalize(L + V);

    float specularFactor = pow(
        max(dot(N, H), 0.0),
        u_Material.Shininess);

    vec3 diffuse = baseColor * nDotL;

    vec3 specular = vec3(
        u_Material.SpecularStrength *
        specularFactor *
        nDotL);

    return (diffuse + specular) * incidentLight;
}

float SpotAttenuation(
    float cosTheta,
    float innerCos,
    float outerCos)
{
    // 内外锥角相同时，使用硬边缘。
    if (innerCos == outerCos)
        return step(outerCos, cosTheta);

    return smoothstep(
        outerCos,
        innerCos,
        cosTheta);
}

void main()
{
    vec3 baseColor =
        u_Material.BaseColor.rgb *
        texture(u_BaseColorMap, v_TexCoord).rgb;

    vec3 N = SafeNormalize(v_WorldNormal);

    vec3 V = SafeNormalize(
        u_CameraPosition - v_WorldPosition);

    vec3 result =
        baseColor *
        u_AmbientColor *
        u_AmbientIntensity;

    if (u_EnvironmentEnabled != 0)
    {
        // The precomputed map stores irradiance / pi, ready for Lambert diffuse.
        vec3 diffuseEnvironment = SampleEnvironment(u_IrradianceMap, N);
        vec3 reflectedDirection = reflect(-V, N);
        vec3 reflectedEnvironment = SampleEnvironment(
            u_EnvironmentMap, reflectedDirection);

        result += u_EnvironmentIntensity * (
            baseColor * diffuseEnvironment +
            reflectedEnvironment * u_ReflectionStrength *
            u_Material.SpecularStrength);
    }

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

        result += EvaluateBlinnPhong(
            baseColor,
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

        result += EvaluateBlinnPhong(
            baseColor,
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

        // -L 为光源指向片元的方向。
        float cosTheta =
            dot(-L, light.Direction);

        float coneAttenuation =
            SpotAttenuation(
                cosTheta,
                light.InnerCos,
                light.OuterCos);

        float attenuation =
            DistanceAttenuation(
                distanceSquared,
                light.Range) *
            coneAttenuation;

        vec3 incidentLight =
            light.Color *
            light.Intensity *
            attenuation;

        result += EvaluateBlinnPhong(
            baseColor,
            N,
            V,
            L,
            incidentLight);
    }

    // 第一版直接输出到窗口，按不透明模型处理。
    o_Color = vec4(result, 1.0);
}
