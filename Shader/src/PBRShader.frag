#version 450
#extension GL_KHR_vulkan_glsl : enable

#define PI (3.14159265359)

layout(location=0) in vec3 fragColor;
layout(location=1) in vec3 fragPos;
layout(location=2) in vec3 fragNormal;

layout(location=0) out vec4 outColor;

struct LightData {
    vec3 pos;
    vec3 color;
};

layout(binding=0) uniform UBO{
    mat4 model;
    mat4 modelNormal;
    mat4 view;
    mat4 perspective;
    vec3 color;
    uint lightCount;
} ubo;

layout(set=0, binding=1) readonly buffer LightBuffer{
    LightData data[];
} lights;

vec3 lambertian(LightData light){
        vec3 vL = light.pos - fragPos;
        vec3 n = fragNormal;
        vec3 E = light.color/pow(length(vL), 2)*dot(n, vL);
        vec3 L = 1/PI*E;
        if(L.x<0) L.x=0;
        if(L.y<0) L.y=0;
        if(L.z<0) L.z=0;
        return L;
}

void main(){
    vec3 light = vec3(0.1, 0.1, 0.1);
    for(uint i=0; i<ubo.lightCount; i++){

        light += lambertian(lights.data[i]);
    }
    vec4 test = vec4(1, 0, 0, 0) * ubo.model;
    outColor = vec4(fragColor*light, 1);
}