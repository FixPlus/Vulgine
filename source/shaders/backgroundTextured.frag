#version 450


layout (binding = 0, set = 0) uniform sampler2D colorMap;

layout (location = 0) in vec3 inUVW;

layout (location = 0) out vec4 outFragColor;


void main(){
    vec4 color = vec4(0.2f, 0.7f, 0.7f,1.0f);
    vec4 textColor = texture(colorMap, vec2(inUVW.x, min(inUVW.y + 0.15f, 1.0f)), 1.0f);

    color = vec4(color.x * textColor.x, color.y * textColor.y,color.z * textColor.z,color.a * textColor.a);
    vec4 mix = vec4(1.0f, 1.0f, 1.0f, 1.0f) - color; //texture(samplerColor1, vec2(inUVW.x, inUVW.y), 1.0f);
    color = color + mix * inUVW.y;
    outFragColor = color;
}