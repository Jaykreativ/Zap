#version 460
#extension GL_KHR_vulkan_glsl : enable

out gl_PerVertex {
	vec4 gl_Position;
};

layout(location=0) in vec3 vertPos;
layout(location=1) in vec3 vertColor;

layout(location=0) out vec4 fragColor;

layout(binding=0) uniform UBO{
	mat4 perspective;
	mat4 view;
} ubo;

void main(){
	fragColor = vec4(vertColor, 1);

	vec4 worldPos = vec4(vertPos, 1);
	vec4 viewPos  = ubo.view * worldPos;
	gl_Position   = ubo.perspective * vec4(viewPos.x, -viewPos.y, viewPos.z, 1);
}