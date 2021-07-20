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

layout(binding = 0, set = 2) uniform sampler2D someMap;

layout(binding = 1, set = 2) uniform UBO
{
    vec4 shift;
} ubo;

void main() {
    //mat4 model = mat4(1.0);
    vec4 temp = (model * vec4(inPos, 1.0f)) + ubo.shift;

    vec2 UV = vec2(temp.z, 100.0f - temp.y) / 100.0;

    temp += vec4(-length(texture(someMap, UV)) * 10.0f, 0.0f, 0.0f, 0.0f);

    outPos = temp.xyz;

    outViewPos = outPos - PushConstants.cameraPosition;

    gl_Position = PushConstants.viewMatrix * temp;
    outNorm = (model * vec4(inNorm, 0.0f)).xyz;

    outUV = inUV;
}