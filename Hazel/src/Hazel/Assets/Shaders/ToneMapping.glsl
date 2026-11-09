#type vertex
#version 330 core

layout(location = 0) in vec2 a_Position;

out vec2 v_TexCoord;

void main()
{
    v_TexCoord = a_Position * 0.5 + 0.5;
    gl_Position = vec4(a_Position, 0.0, 1.0);
}


#type fragment
#version 330 core

layout(location = 0) out vec4 o_Color;

in vec2 v_TexCoord;

uniform sampler2D u_HDRColor;
uniform float u_Exposure;


// 指数色调映射。
// 将非负 HDR 颜色压缩到显示范围。
vec3 ToneMap(vec3 color)
{
    return vec3(1.0) - exp(-color * u_Exposure);
}


// 将线性颜色编码为 sRGB。
vec3 LinearToSRGB(vec3 color)
{
    vec3 low = color * 12.92;

    vec3 high =
        1.055 * pow(color, vec3(1.0 / 2.4)) - 0.055;

    return mix(
        high,
        low,
        lessThanEqual(color, vec3(0.0031308)));
}


void main()
{
    vec3 hdrColor = texture(u_HDRColor, v_TexCoord).rgb;

    // 保证后续指数映射和幂运算使用非负颜色。
    hdrColor = max(hdrColor, vec3(0.0));

    vec3 mappedColor = ToneMap(hdrColor);

    vec3 displayColor = LinearToSRGB(mappedColor);

    o_Color = vec4(displayColor, 1.0);
}