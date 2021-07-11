#version 450

layout (location = 0) in vec4 inColor;
layout (location = 1) in vec4 inPos;
layout (location = 3) in vec2 inUV;

layout (location = 0) out vec4 outFragColor;

layout(binding = 0, set = 0) uniform sampler2D colorMap;

void main()
{
    //float brightness = length(inPos - vec4(0.0));
    //brightness *= brightness;
    outFragColor = texture(colorMap, inUV);//* brightness;
}