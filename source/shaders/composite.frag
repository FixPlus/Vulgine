#version 450

layout (input_attachment_index = 0, binding = 0) uniform subpassInput samplerposition;
layout (input_attachment_index = 1, binding = 1) uniform subpassInput samplerNormal;
layout (input_attachment_index = 2, binding = 2) uniform subpassInput samplerAlbedo;

layout (location = 0) in vec3 inUVW;

layout (location = 0) out vec4 outColor;

struct Data{
    vec4 color;
    vec4 direction;
};

layout(binding = 3, set = 0) uniform UBO{
    Data data[20];
    uint count;
} lights;

layout(push_constant) uniform constants{
    mat4 viewMatrix;
    vec3 cameraPosition;
} PushConstants;

void main() {
    // Read G-Buffer values from previous sub pass
    vec3 fragPos = subpassLoad(samplerposition).rgb;
    vec3 normal = subpassLoad(samplerNormal).rgb;
    vec4 albedo = subpassLoad(samplerAlbedo);
/*
    #define ambient 0.15

    // Ambient part
    vec3 fragcolor  = albedo.rgb * ambient;

    for(int i = 0; i < count; ++i)
    {
        // Vector to light
        vec3 L = ubo.lights[i].position.xyz - fragPos;
        // Distance from light to fragment position
        float dist = length(L);

        // Viewer to fragment
        vec3 V = ubo.viewPos.xyz - fragPos;
        V = normalize(V);

        // Light to fragment
        L = normalize(L);

        // Attenuation
        float atten = ubo.lights[i].radius / (pow(dist, 2.0) + 1.0);

        // Diffuse part
        vec3 N = normalize(normal);
        float NdotL = max(0.0, dot(N, L));
        vec3 diff = ubo.lights[i].color * albedo.rgb * NdotL * atten;

        // Specular part
        // Specular map values are stored in alpha of albedo mrt
        vec3 R = reflect(-L, N);
        float NdotR = max(0.0, dot(R, V));
        //vec3 spec = ubo.lights[i].color * albedo.a * pow(NdotR, 32.0) * atten;

        fragcolor += diff;// + spec;
    }
        outColor = vec4(fragcolor, 1.0);
*/
    vec4 totalBrightness = vec4(0.0f, 0.0f, 0.0f, 1.0f);
    vec4 totalSpecular =  vec4(0.0f, 0.0f, 0.0f, 0.0f);
    for(uint i = 0; i < lights.count; i++){
        vec3 light = normalize(lights.data[i].direction.xyz);
        float diffuse = dot(-light, normalize(normal)) * 0.4f + 0.6f;
        totalBrightness += lights.data[i].color * diffuse;
        totalSpecular += lights.data[i].color * pow(max(dot(reflect(normalize(fragPos - PushConstants.cameraPosition), normal), -light), 0.0f), 16.0f);

    }

    totalBrightness.w = 1.0f;
    totalSpecular.w = 0.0f;
    totalSpecular *= albedo.a;
    outColor =  vec4(albedo.rgb, 1.0f) * totalBrightness + totalSpecular;


}
