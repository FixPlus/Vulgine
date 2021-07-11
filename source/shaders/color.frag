#version 450


layout (location = 0) in vec3 inPos;
layout (location = 1) in vec3 inNorm;
layout (location = 2) in vec2 inUV;
layout (location = 3) in vec3 inViewVec;

layout (location = 0) out vec4 outFragColor;

layout(binding = 0, set = 0) uniform sampler2D colorMap;

void main()
{
    vec3 light = normalize(vec3(0.0f, 0.0f, 1.0f));
    float brightness = 0.6f + 0.4f * dot(-light, normalize(inNorm));
    vec4 specular = vec4(1.0f, 1.0f, 1.0f, 0.0f) * pow(max(dot(reflect(normalize(inViewVec) , inNorm), -light), 0.0f), 16.0f);

    outFragColor = texture(colorMap, inUV) * brightness + specular;
}