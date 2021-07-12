#version 450

layout(location = 0) out vec3 outPos;
layout(location = 1) out vec3 outNorm;
layout(location = 2) out vec2 outUV;
layout(location = 3) out vec3 outViewPos;

layout(location = 0) in vec3 inPos;
layout(location = 1) in vec3 inNorm;
layout(location = 2) in vec2 inUV;

layout(location = 3) in mat4 model;

layout(push_constant) uniform constants{
    mat4 viewMatrix;
    vec3 cameraPosition;
} PushConstants;


void main() {
    //mat4 model = mat4(1.0);
    outPos = (model * vec4(inPos, 1.0f)).xyz;

    outViewPos = outPos - PushConstants.cameraPosition;

    gl_Position = PushConstants.viewMatrix * model * vec4(inPos, 1.0f);
    outNorm = (model * vec4(inNorm, 0.0f)).xyz;

    outUV = inUV;
}