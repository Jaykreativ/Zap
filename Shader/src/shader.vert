#version 450
#extension GL_KHR_vulkan_glsl : enable

out gl_PerVertex {
	vec4 gl_Position;
};

layout(location=0) in vec3 pos;

layout(location=0) out vec3 fragColor;

layout(binding=0) uniform UBO{
    mat4 model;
    mat4 view;
    mat4 perspective;
    vec3 color;
} ubo;

void main(){
    gl_Position = ubo.perspective * ubo.view * ubo.model * vec4(pos, 1);
    fragColor = ubo.color;
}