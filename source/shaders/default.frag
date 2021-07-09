#version 450

layout (location = 0) in vec4 inColor;
layout (location = 1) in vec4 inPos;

layout (location = 0) out vec4 outFragColor;

void main()
{
    //float brightness = length(inPos - vec4(0.0));
    //brightness *= brightness;
    outFragColor = vec4(inColor);//* brightness;
}