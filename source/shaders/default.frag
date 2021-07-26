#version 450

layout (location = 0) in vec3 inPos;
layout (location = 1) in vec3 inNorm;
layout (location = 2) in vec2 inUV;
layout (location = 3) in vec3 inViewVec;
layout (location = 4) in vec3 inColor;

layout (location = 0) out vec4 outFragColor;

layout(binding = 0, set = 0) uniform UBO{
    vec4 baseColor;
    float specular;
}   material;

void main()
{

    if(material.baseColor.a == 0.0f){
        discard;
    }

    //float brightness = length(inPos - vec4(0.0));
    //brightness *= brightness;
    outFragColor = vec4(inColor, 1.0f) * material.baseColor;//* brightness;
}