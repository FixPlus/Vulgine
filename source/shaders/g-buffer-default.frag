#version 450


layout (location = 0) in vec3 inPos;
layout (location = 1) in vec3 inNorm;
layout (location = 2) in vec2 inUV;
layout (location = 3) in vec3 inViewVec;
layout (location = 4) in vec3 inColor;

layout (location = 0) out vec4 outColor;
layout (location = 1) out vec4 outPosition;
layout (location = 2) out vec4 outNormal;
layout (location = 3) out vec4 outAlbedo;

layout (constant_id = 0) const float NEAR_PLANE = 0.1f;
layout (constant_id = 1) const float FAR_PLANE = 256.0f;


layout(binding = 0, set = 0) uniform UBO{
    vec4 baseColor;
    float specular;
}   material;

float linearDepth(float depth)
{
    float z = depth * 2.0f - 1.0f;
    return (2.0f * NEAR_PLANE * FAR_PLANE) / (FAR_PLANE + NEAR_PLANE - z * (FAR_PLANE - NEAR_PLANE));
}

void main()
{
    outPosition = vec4(inPos, 1.0);

    vec3 N = normalize(inNorm);
    N.y = -N.y;
    outNormal = vec4(N, 1.0);
    // Write color attachments to avoid undefined behaviour (validation error)
    outColor = vec4(0.0);

    if(material.baseColor.a == 0.0f){
        discard;
    }

    outAlbedo.rgb = inColor * material.baseColor.rgb;
    outAlbedo.a = material.specular;

    // Store linearized depth in alpha component
    outPosition.a = linearDepth(gl_FragCoord.z);


}