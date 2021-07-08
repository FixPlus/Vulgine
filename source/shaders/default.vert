#version 450

layout(location = 0) out vec4 fragColor;

layout(location = 0) in vec4 inPos;
layout(location = 1) in vec4 inCol;


void main() {
    gl_Position = inPos + vec4(0.0f , 0.0f, 0.5f, 1.0f);
    fragColor = inCol;
}