#version 460
#extension GL_KHR_vulkan_glsl : enable

#extension GL_EXT_shader_explicit_arithmetic_types_int64 : require

out gl_PerVertex {
	vec4 gl_Position;
};

layout(location=0) in vec3 vertPos;
layout(location=1) in vec2 texCoords;
layout(location=2) in vec3 vertNormal;

layout(location=0) out vec3 fragPos;
layout(location=1) out vec2 fragTexCoords;
layout(location=2) out vec3 fragNormal;

layout( push_constant ) uniform PushConstants {
    uint instanceIndex;
} constants;

layout(binding=0) uniform UBO{
    mat4 model;
    mat4 modelNormal;
    mat4 view;
    mat4 perspective;
    vec3 color;
    uint lightCount;
} ubo;

struct Material {
    vec3 albedo;
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
    uint64_t vertexAddress;
	uint64_t indexAddress;
};

layout(set=0, binding=2) readonly buffer PerMeshInstanceBuffer{
    PerMeshInstanceData data[];
} perMeshInstance;

void main(){
    vec4 worldPos = perMeshInstance.data[constants.instanceIndex].transform * vec4(vertPos, 1);
    vec4 viewPos = ubo.view * worldPos;
    gl_Position = ubo.perspective * vec4(viewPos.x, -viewPos.y, viewPos.z, 1);
    fragPos = vec3(worldPos);
    fragTexCoords = texCoords;
    fragNormal = vec3(perMeshInstance.data[constants.instanceIndex].normalTransform * vec4(vertNormal, 0));
}