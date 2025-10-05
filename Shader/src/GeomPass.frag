#version 460
#extension GL_KHR_vulkan_glsl : enable
#extension GL_EXT_nonuniform_qualifier : enable

#define PI (3.14159265359)

layout(location=0) in vec3 fragPos;
layout(location=1) in vec2 fragTexCoords;
layout(location=2) in vec3 fragNormal;

layout(location=0) out vec4 outColor;

layout( push_constant ) uniform PushConstants {
	uint instanceIndex;
} constants;

layout(binding=0) uniform UBO{
	mat4 view;
	mat4 perspective;
} ubo;

struct Material {
	vec4 albedo;
	uint albedoMap;
	float metallic;
	uint metallicMap;
	float roughness;
	uint roughnessMap;
	vec4 emissive;
	uint emissiveMap;
};

struct PerMeshInstanceData {
	mat4 transform;
	mat4 normalTransform;
	Material material;
	int unused[4];
};

layout(set=0, binding=2) readonly buffer PerMeshInstanceBuffer {
	PerMeshInstanceData data[];
} perMeshInstance;

layout(set=1, binding=0) uniform sampler2D textures[];

void main(){
	Material material = perMeshInstance.data[constants.instanceIndex].material;
	vec4 albedo = material.albedo;
	if(material.albedoMap < 0xFFFFFFFF)
		albedo *= texture(textures[int(material.albedoMap)], fragTexCoords);
	float metallic = material.metallic;
	if(material.metallicMap < 0xFFFFFFFF)
		metallic *= texture(textures[int(material.metallicMap)], fragTexCoords).b;
	float roughness = material.roughness;
	if(material.roughnessMap < 0xFFFFFFFF)
		roughness *= texture(textures[int(material.roughnessMap)], fragTexCoords).g;
	vec4 emissive = material.emissive;
	if(material.emissiveMap < 0xFFFFFFFF)
		emissive *= vec4(texture(textures[int(material.emissiveMap)], fragTexCoords).xyz, 1);

	outColor = albedo;
}