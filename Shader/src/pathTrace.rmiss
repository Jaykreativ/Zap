#version 460
#extension GL_EXT_ray_tracing : require

layout(location = 0) rayPayloadInEXT Payload{
	uint recursionDepth;
	uint randomSeed;
	vec3 rayOrigin;
	vec3 rayDirection;
	vec3 radiance;
} prd;

void main() {
	prd.radiance = vec3(0, 0, 0);
}