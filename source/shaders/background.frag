#version 450

layout (location = 0) in vec3 inUVW;

layout (location = 0) out vec4 outFragColor;


void main(){
    vec4 color = vec4(0.2f, 0.7f, 0.7f,1.0f);

    outFragColor = color;
}