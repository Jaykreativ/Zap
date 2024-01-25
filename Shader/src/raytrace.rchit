#version 460
#extension GL_EXT_ray_tracing : require

hitAttributeEXT vec2 attribs;

layout(location = 0) rayPayloadInEXT vec3 prd;

void main() {
	prd = vec3(attribs, 0);
}