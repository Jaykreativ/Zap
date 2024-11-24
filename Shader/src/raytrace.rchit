#version 460
#extension GL_EXT_nonuniform_qualifier : enable

#extension GL_EXT_ray_tracing : require
#extension GL_EXT_shader_explicit_arithmetic_types_int64 : require
#extension GL_EXT_buffer_reference2 : require

#define PI (3.14159265359)

hitAttributeEXT vec2 attribs;

layout(location = 0) rayPayloadInEXT vec3 prd;
layout(location = 1) rayPayloadEXT bool isShadowed;

layout(set = 0, binding = 0) uniform accelerationStructureEXT accelerationStructure;

const int vertexSize = 8; // in floats
struct Vertex {
	vec3 pos;
	vec2 texCoords;
	vec3 normal;
};

layout(buffer_reference) buffer Vertices { float v[]; };
layout(buffer_reference) buffer Indices { uint i[]; };

layout(set=1, binding = 0) uniform UBO{
	mat4 inverseView;
	mat4 inversePerspective;
	vec3 camPos;
	uint lightCount;
} ubo;

// Shared Buffers
struct LightData {
	vec3 pos;
	vec3 color;
	float strength;
	float radius;
};

layout(set=1, binding=1) readonly buffer LightBuffer {
	LightData data[];
} lightBuffer;

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
	uint64_t vertexAddress;
	uint64_t indexAddress;
};

layout(set = 1, binding = 2) readonly buffer PerMeshInstanceBuffer {
	PerMeshInstanceData data[];
} perMeshInstanceBuffer;

layout(set = 1, binding = 3) uniform sampler2D textures[];

vec3 lambertian(LightData light, vec3 vL, vec3 normal){
		vec3 n        = normal;
		vec3 E        = light.color/pow(length(vL), 2)*dot(n, vL);
		vec3 L        = 1/PI*E;
		L = max(L, vec3(0));
		return L;
}

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


void main() {
	PerMeshInstanceData perMeshInstanceData = perMeshInstanceBuffer.data[gl_InstanceCustomIndexEXT];
	Vertices vertices    = Vertices(perMeshInstanceData.vertexAddress);
	Indices indices      = Indices(perMeshInstanceData.indexAddress);
	
	uint ind0 = indices.i[gl_PrimitiveID*3+0];
	uint ind1 = indices.i[gl_PrimitiveID*3+1];
	uint ind2 = indices.i[gl_PrimitiveID*3+2];
	
	// Vertex of the triangle
	Vertex v0 = Vertex(
		vec3(vertices.v[ind0*vertexSize+0], vertices.v[ind0*vertexSize+1], vertices.v[ind0*vertexSize+2]),
		vec2(vertices.v[ind0*vertexSize+3], vertices.v[ind0*vertexSize+4]),
		vec3(vertices.v[ind0*vertexSize+5], vertices.v[ind0*vertexSize+6], vertices.v[ind0*vertexSize+7])
	);
	Vertex v1 = Vertex(
		vec3(vertices.v[ind1*vertexSize+0], vertices.v[ind1*vertexSize+1], vertices.v[ind1*vertexSize+2]),
		vec2(vertices.v[ind1*vertexSize+3], vertices.v[ind1*vertexSize+4]),
		vec3(vertices.v[ind1*vertexSize+5], vertices.v[ind1*vertexSize+6], vertices.v[ind1*vertexSize+7])
	);
	Vertex v2 = Vertex(
		vec3(vertices.v[ind2*vertexSize+0], vertices.v[ind2*vertexSize+1], vertices.v[ind2*vertexSize+2]),
		vec2(vertices.v[ind2*vertexSize+3], vertices.v[ind2*vertexSize+4]),
		vec3(vertices.v[ind2*vertexSize+5], vertices.v[ind2*vertexSize+6], vertices.v[ind2*vertexSize+7])
	);
	
	const vec3 barycentrics = vec3(1.0 - attribs.x - attribs.y, attribs.x, attribs.y);
	
	// Computing the coordinates of the hit position
	const vec3 pos      = v0.pos * barycentrics.x + v1.pos * barycentrics.y + v2.pos * barycentrics.z;
	const vec3 worldPos = vec3(gl_ObjectToWorldEXT * vec4(pos, 1.0));  // Transforming the position to world space

	const vec2 texCoords = v0.texCoords * barycentrics.x + v1.texCoords * barycentrics.y + v2.texCoords * barycentrics.z;
	
	const vec3 normal      = v0.normal * barycentrics.x + v1.normal * barycentrics.y + v2.normal * barycentrics.z;
	const vec3 worldNormal = normalize(vec3(normal * gl_WorldToObjectEXT));  // Transforming the normal to world space
	
	Material material = perMeshInstanceData.material;
	vec4 albedo = material.albedo;
	if(material.albedoMap < 0xFFFFFFFF)
		albedo *= texture(textures[int(material.albedoMap)], texCoords);
	float metallic = material.metallic;
	if(material.metallicMap < 0xFFFFFFFF)
		metallic *= texture(textures[int(material.metallicMap)], texCoords).b;
	float roughness = material.roughness;
	if(material.roughnessMap < 0xFFFFFFFF)
		roughness *= texture(textures[int(material.roughnessMap)], texCoords).g;
	vec4 emissive = material.emissive;
	if(material.emissiveMap < 0xFFFFFFFF)
		emissive *= vec4(texture(textures[int(material.emissiveMap)], texCoords).xyz, 1);

	uint rayFlags = gl_RayFlagsSkipClosestHitShaderEXT | gl_RayFlagsOpaqueEXT | gl_RayFlagsTerminateOnFirstHitEXT;
	float tMin = 0.001;

	vec3 N = normalize(worldNormal);
	vec3 V = normalize(ubo.camPos - worldPos);

	vec3 F0 = vec3(0.04); 
	F0 = mix(F0, albedo.rgb, metallic);
			   
	// reflectance equation
	vec3 Lo = vec3(0);
	for(int i = 0; i < 4; ++i) 
	{
		// calculate per-light radiance
		vec3 L = normalize(lightBuffer.data[i].pos - worldPos);
		float r = lightBuffer.data[i].radius;
		float distance    = max(length(lightBuffer.data[i].pos - worldPos)-r, 0.0001);
		isShadowed = true;
		if(dot(worldNormal, L) > 0){
			traceRayEXT(accelerationStructure,    // acceleration structure
						rayFlags,                 // rayFlags
						0xFF,                     // cullMask
						0,                        // sbtRecordOffset
						0,                        // sbtRecordStride
						1,                        // missIndex
						worldPos,                 // ray origin
						tMin,                     // ray min range
						L,                        // ray direction
						distance,                 // ray max range
						1                         // payload (location = 1)
			);
		}
		Lo += emissive.xyz * emissive.w;
		if(isShadowed) continue;
		
		vec3 H = normalize(V + L);
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
		Lo += (kD * albedo.rgb / PI + specular) * radiance * NdotL; 
	}

	vec3 color = Lo;
	
	color = color / (color + vec3(1.0));
	color = pow(color, vec3(1.0/2.2));  

	prd = color;
}