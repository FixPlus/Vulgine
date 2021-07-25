#version 450


layout (location = 0) in vec3 inPos;
layout (location = 1) in vec3 inNorm;
layout (location = 2) in vec2 inUV;
layout (location = 3) in vec3 inViewVec;
layout (location = 4) in vec3 inColor;

layout (location = 0) out vec4 outFragColor;

layout(binding = 0, set = 0) uniform sampler2D colorMap;

layout(binding = 1, set = 0) uniform UBO{
    float specular;
}   ubo;

void main()
{
    vec4 totalBrightness = vec4(1.0f, 1.0f, 1.0f, 1.0f);
    vec4 totalSpecular =  vec4(0.0f, 0.0f, 0.0f, 0.0f);

    vec3 light = vec3(0.0f, 1.0f, 0.0f);

    float diffuse = dot(-light, normalize(inNorm)) * 0.4f + 0.6f;
    totalBrightness *= diffuse;
    totalSpecular += vec4(1.0f) * pow(max(dot(reflect(normalize(inViewVec), inNorm), -light), 0.0f), 16.0f);

    totalBrightness.w = 1.0f;
    totalSpecular.w = 0.0f;

    outFragColor = vec4(inColor, 1.0f) * texture(colorMap, inUV) * totalBrightness + ubo.specular * totalSpecular;
}