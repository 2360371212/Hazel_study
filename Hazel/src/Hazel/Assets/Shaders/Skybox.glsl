#type vertex
#version 330 core

layout(location = 0) in vec2 a_Position;

out vec2 v_NDC;

void main()
{
    v_NDC = a_Position;

    gl_Position = vec4(
        a_Position,
        0.0,
        1.0);
}


#type fragment
#version 330 core

layout(location = 0) out vec4 o_Color;

in vec2 v_NDC;

uniform samplerCube u_EnvironmentMap;

uniform mat4 u_InverseProjection;
uniform mat3 u_InverseViewRotation;

uniform float u_EnvironmentRotation;
uniform float u_Intensity;


// 绕世界 Y 轴旋转环境采样方向。
// u_EnvironmentRotation 使用弧度。
vec3 RotateEnvironment(vec3 direction)
{
    float c = cos(u_EnvironmentRotation);
    float s = sin(u_EnvironmentRotation);

    return vec3(
        c * direction.x + s * direction.z,
        direction.y,
        -s * direction.x + c * direction.z);
}


void main()
{
    // 从屏幕位置重建相机空间中的观察方向。
    vec4 viewPosition =
        u_InverseProjection * vec4(v_NDC, 1.0, 1.0);

    // 仅使用相机旋转，让天空不受相机平移影响。
    vec3 direction = normalize(
        u_InverseViewRotation * viewPosition.xyz);

    direction = RotateEnvironment(direction);

    vec3 radiance =
        texture(u_EnvironmentMap, direction).rgb * u_Intensity;

    // 保留超过 1.0 的亮度，写入浮点 HDR 附件。
    o_Color = vec4(radiance, 1.0);
}
