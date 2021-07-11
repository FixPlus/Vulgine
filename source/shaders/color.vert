#version 450

layout(location = 0) out vec4 fragColor;
layout(location = 1) out vec4 outPos;
layout(location = 3) out vec2 outUV;

layout(location = 0) in vec4 inPos;
layout(location = 1) in vec4 inCol;
layout(location = 2) in vec2 inUV;

layout(push_constant) uniform constants{
    mat4 viewMatrix;
} PushConstants;


void main() {
    vec4 cameraSpacePos = PushConstants.viewMatrix * inPos;
    gl_Position = cameraSpacePos;
    fragColor = inCol;
    outPos = cameraSpacePos;

    outUV = inUV;
}