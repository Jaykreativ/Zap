#version 450
#extension GL_KHR_vulkan_glsl : enable

layout(location=0) in vec3 fragColor;
layout(location=1) in vec3 fragPos;

layout(location=0) out vec4 outColor;

struct LightData {
    vec3 pos;
    vec3 color;
};

layout(binding=0) uniform UBO{
    mat4 model;
    mat4 view;
    mat4 perspective;
    vec3 color;
    uint lightCount;
} ubo;

layout(set=0, binding=1) readonly buffer LightBuffer{
    LightData data[];
} lights;

void main(){
    vec3 light = vec3(0, 0, 0);
    for(uint i=0; i<ubo.lightCount; i++){
        light += lights.data[i].color / length(fragPos - lights.data[i].pos);
    }
    outColor = vec4(fragColor*light, 1);
}