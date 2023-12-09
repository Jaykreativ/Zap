#version 450
#extension GL_KHR_vulkan_glsl : enable

out gl_PerVertex {
	vec4 gl_Position;
};

layout(location=0) in vec3 vertPos;
layout(location=1) in vec3 vertNormal;

layout(location=0) out vec3 fragColor;
layout(location=1) out vec3 fragPos;
layout(location=2) out vec3 fragNormal;

layout(binding=0) uniform UBO{
    mat4 model;
    mat4 modelNormal;
    mat4 view;
    mat4 perspective;
    vec3 color;
    uint lightCount;
} ubo;

struct PerMeshData {
    mat4 transform;
    mat4 normalTransform;
    vec4 color;
};

layout(set=0, binding=2) readonly buffer PerMeshBuffer{
    PerMeshData data[];
} perMesh;

layout( push_constant ) uniform PushConstant {
    uint index;
}constants;

void main(){
    vec4 worldPos = perMesh.data[constants.index].transform * vec4(vertPos, 1);
    vec4 viewPos = ubo.view * worldPos;
    gl_Position = ubo.perspective * vec4(viewPos.x, -viewPos.y, viewPos.z, 1);
    fragColor = vec3(perMesh.data[constants.index].color);
    fragPos = vec3(worldPos);
    fragNormal = vec3(perMesh.data[constants.index].normalTransform * vec4(vertNormal, 0));
}