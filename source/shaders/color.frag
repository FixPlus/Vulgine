#version 450


layout (location = 0) in vec3 inPos;
layout (location = 1) in vec3 inNorm;
layout (location = 2) in vec2 inUV;
layout (location = 3) in vec3 inViewVec;
layout (location = 4) in vec3 inColor;

layout (location = 0) out vec4 outFragColor;

layout(binding = 0, set = 1) uniform sampler2D colorMap;
struct Data{
    vec4 color;
    vec4 direction;
};
layout(binding = 0, set = 0) uniform UBO{
    Data data[20];
    uint count;
} lights;

void main()
{
    vec4 totalBrightness = vec4(0.0f, 0.0f, 0.0f, 1.0f);
    vec4 totalSpecular =  vec4(0.0f, 0.0f, 0.0f, 0.0f);
    for(uint i = 0; i < lights.count; i++){
        vec3 light = normalize(lights.data[i].direction.xyz);
        float diffuse = dot(-light, normalize(inNorm)) * 0.4f + 0.6f;
        totalBrightness += lights.data[i].color * diffuse;
        totalSpecular += lights.data[i].color * pow(max(dot(reflect(normalize(inViewVec), inNorm), -light), 0.0f), 16.0f);

    }
    totalBrightness.w = 1.0f;
    totalSpecular.w = 0.0f;
    outFragColor = vec4(inColor, 1.0f) * texture(colorMap, inUV) * totalBrightness + totalSpecular;
}