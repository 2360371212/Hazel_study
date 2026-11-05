#type vertex
#version 330 core

layout(location = 0) in vec2 a_Position;

out vec2 v_NDC;

void main()
{
    v_NDC = a_Position;
    gl_Position = vec4(a_Position, 0.0, 1.0);
}

#type fragment
#version 330 core

layout(location = 0) out vec4 o_Color;

in vec2 v_NDC;

uniform sampler2D u_EnvironmentMap;
uniform mat4 u_InverseProjection;
uniform mat3 u_InverseViewRotation;
uniform float u_EnvironmentRotation;
uniform float u_Intensity;

vec3 EnvironmentTexel(ivec2 position)
{
    ivec2 size = textureSize(u_EnvironmentMap, 0);
    position.x = (position.x % size.x + size.x) % size.x;
    position.y = clamp(position.y, 0, size.y - 1);
    return texelFetch(u_EnvironmentMap, position, 0).rgb;
}

void main()
{
    vec4 viewPosition = u_InverseProjection * vec4(v_NDC, 1.0, 1.0);
    vec3 direction = normalize(u_InverseViewRotation * viewPosition.xyz);

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

    vec2 pixel = uv * vec2(textureSize(u_EnvironmentMap, 0)) - 0.5;
    ivec2 base = ivec2(floor(pixel));
    vec2 weight = fract(pixel);

    vec3 bottom = mix(
        EnvironmentTexel(base),
        EnvironmentTexel(base + ivec2(1, 0)), weight.x);
    vec3 top = mix(
        EnvironmentTexel(base + ivec2(0, 1)),
        EnvironmentTexel(base + ivec2(1, 1)), weight.x);

    o_Color = vec4(mix(bottom, top, weight.y) * u_Intensity, 1.0);
}
