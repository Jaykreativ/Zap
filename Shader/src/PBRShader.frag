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
	mat4 model;
	mat4 modelNormal;
	mat4 view;
	mat4 perspective;
	vec3 camPos;
	vec3 color;
	uint lightCount;
} ubo;

struct LightData {
	vec3 pos;
	vec3 color;
	float strength;
	float radius;
};

layout(set=0, binding=1) readonly buffer LightBuffer{
	LightData data[];
} lightBuffer;

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
	int unused[4];
};

layout(set=0, binding=2) readonly buffer PerMeshInstanceBuffer {
	PerMeshInstanceData data[];
} perMeshInstance;

layout(set=0, binding=3) uniform sampler2D textures[];

vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
	return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}  

float DistributionGGX(vec3 N, vec3 H, float roughness)
{
	float a      = roughness*roughness;
	float a2     = a*a;
	float NdotH  = max(dot(N, H), 0.0);
	float NdotH2 = NdotH*NdotH;
	
	float num   = a2;
	float denom = (NdotH2 * (a2 - 1.0) + 1.0);
	denom = PI * denom * denom;
	
	return num / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
	float r = (roughness + 1.0);
	float k = (r*r) / 8.0;

	float num   = NdotV;
	float denom = NdotV * (1.0 - k) + k;
	
	return num / denom;
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
	float NdotV = max(dot(N, V), 0.0);
	float NdotL = max(dot(N, L), 0.0);
	float ggx2  = GeometrySchlickGGX(NdotV, roughness);
	float ggx1  = GeometrySchlickGGX(NdotL, roughness);
	
	return ggx1 * ggx2;
}

void main(){
	Material material = perMeshInstance.data[constants.instanceIndex].material;
	vec3 albedo = material.albedo;
	if(material.albedoMap < 0xFFFFFFFF)
		albedo *= texture(textures[material.albedoMap], fragTexCoords).rgb;
	float metallic = material.metallic;
	if(material.metallicMap < 0xFFFFFFFF)
		metallic *= texture(textures[material.metallicMap], fragTexCoords).b;
	float roughness = material.roughness;
	if(material.roughnessMap < 0xFFFFFFFF)
		roughness *= texture(textures[material.roughnessMap], fragTexCoords).g;
	vec4 emissive = material.emissive;
	if(material.emissiveMap < 0xFFFFFFFF)
		emissive *= vec4(texture(textures[material.emissiveMap], fragTexCoords).xyz, 1);


	vec3 N = normalize(fragNormal);
	vec3 V = normalize(ubo.camPos - fragPos);

	vec3 F0 = vec3(0.04); 
	F0 = mix(F0, albedo, metallic);
			   
	// reflectance equation
	vec3 Lo = vec3(0.0);
	for(int i = 0; i < ubo.lightCount; i++) 
	{
		// calculate per-light radiance
		vec3 L = normalize(lightBuffer.data[i].pos - fragPos);
		vec3 H = normalize(V + L);
		float r = lightBuffer.data[i].radius;
		float distance    = max(length(lightBuffer.data[i].pos - fragPos)-r, 0.0001);
		float attenuation = 1.0 / (distance * distance);
		vec3 radiance     = lightBuffer.data[i].color * lightBuffer.data[i].strength * attenuation;        
		
		// cook-torrance brdf
		float NDF = DistributionGGX(N, H, roughness);        
		float G   = GeometrySmith(N, V, L, roughness);      
		vec3 F    = fresnelSchlick(max(dot(H, V), 0.0), F0);       
		
		vec3 kS = F;
		vec3 kD = vec3(1.0) - kS;
		kD *= 1.0 - metallic;
		
		vec3 numerator    = NDF * G * F;
		float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001;
		vec3 specular     = numerator / denominator;  
			
		// add to outgoing radiance Lo
		float NdotL = max(dot(N, L), 0.0);
		Lo += (kD * albedo / PI + specular) * radiance * NdotL;
	}   
  
	vec3 color = emissive.xyz * emissive.w + Lo;
	
	color = color / (color + vec3(1.0));
	color = pow(color, vec3(1.0/2.2));  
   
	outColor = vec4(color, 1);
}