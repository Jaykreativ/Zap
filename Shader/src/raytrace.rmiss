#version 460
#extension GL_EXT_ray_tracing : require

layout(location = 0) rayPayloadInEXT vec3 prd;

void main() {
    prd = vec3(0, 0, 0);
}