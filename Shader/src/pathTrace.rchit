#version 460
#extension GL_EXT_nonuniform_qualifier : enable

#extension GL_EXT_ray_tracing : require
#extension GL_EXT_shader_explicit_arithmetic_types_int64 : require
#extension GL_EXT_buffer_reference2 : require

#define PI (3.14159265359)

hitAttributeEXT vec2 attribs;

layout(location = 0) rayPayloadInEXT Payload{
	uint recursionDepth;
	uint randomSeed;
	vec3 rayOrigin;
	vec3 rayDirection;
	vec3 radiance;
} prd;

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
	uint lightCount;
	uint frameIndex;
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

layout(set = 1, binding = 2) readonly buffer PerMeshInstanceBuffer {
	PerMeshInstanceData data[];
} perMeshInstanceBuffer;

layout(set = 1, binding = 3) uniform sampler2D textures[];

// Generate a random unsigned int in [0, 2^24) given the previous RNG state
// using the Numerical Recipes linear congruential generator
uint pcgHash(inout uint prev)
{
  uint state = prev * 747796405u + 2891336453u;
  uint word = ((state >> ((state >> 28u)+4u)) ^ state) * 277803737u;
  prev       = (word >> 22u) ^ word;
  return prev & 0x00FFFFFF;
}

// Generate a random float in [0, 1) given the previous RNG state
float rnd(inout uint prev)
{
  return (float(pcgHash(prev)) / float(0x01000000));
}

// Randomly samples from a cosine-weighted hemisphere oriented in the `z` direction.
// From Ray Tracing Gems section 16.6.1, "Cosine-Weighted Hemisphere Oriented to the Z-Axis"
vec3 samplingFacetNormal(inout uint seed, in vec3 x, in vec3 y, in vec3 z, in float roughness)
{
	float r1 = rnd(seed);
	float r2 = rnd(seed);
	float a  = roughness*roughness;
	
	float theta = atan(a*sqrt(r1/(1-r1)));
	float phi = (r2*2-1)*PI;
	
	vec3 direction = vec3(
		sin(theta)*cos(phi),
		sin(theta)*sin(phi),
		cos(theta)
	);
	direction = direction.x * x + direction.y * y + direction.z * z;
	
	return direction;
}

vec3 samplingHemisphere(inout uint seed, in vec3 x, in vec3 y, in vec3 z)
{
  float r1 = rnd(seed);
  float r2 = rnd(seed);
  float sq = sqrt(r1);

  vec3 direction = vec3(cos(2 * PI * r2) * sq, sin(2 * PI * r2) * sq, sqrt(1. - r1));
  direction      = direction.x * x + direction.y * y + direction.z * z;

  return direction;
}

// Return the tangent and binormal from the incoming normal
void createCoordinateSystem(in vec3 N, out vec3 Nt, out vec3 Nb)
{
	if(abs(N.x) > abs(N.y))
		Nt = vec3(N.z, 0, -N.x) / sqrt(N.x * N.x + N.z * N.z);
	else
		Nt = vec3(0, -N.z, N.y) / sqrt(N.y * N.y + N.z * N.z);
	Nb = cross(N, Nt);
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
	if(gl_HitKindEXT == 1){
		prd.radiance = lightBuffer.data[gl_InstanceCustomIndexEXT].color * lightBuffer.data[gl_InstanceCustomIndexEXT].strength;
		return;
	}

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
	vec3 albedo = material.albedo;
	if(material.albedoMap < 0xFFFFFFFF)
		albedo *= texture(textures[material.albedoMap], texCoords).rgb;
	float metallic = material.metallic;
	if(material.metallicMap < 0xFFFFFFFF)
		metallic *= texture(textures[material.metallicMap], texCoords).b;
	float roughness = material.roughness;
	if(material.roughnessMap < 0xFFFFFFFF)
		roughness *= texture(textures[material.roughnessMap], texCoords).g;
	vec4 emissive = material.emissive;
	if(material.emissiveMap < 0xFFFFFFFF)
		emissive *= vec4(texture(textures[material.emissiveMap], texCoords).xyz, 1);

	uint rayFlags = gl_RayFlagsSkipClosestHitShaderEXT | gl_RayFlagsOpaqueEXT | gl_RayFlagsTerminateOnFirstHitEXT;
	float tMin = 0.001f;

	vec3 N = normalize(worldNormal);
	vec3 V = -prd.rayDirection;

	vec3 F0 = vec3(0.04); 
	F0 = mix(F0, albedo, metallic);

	// reflectance equation
	vec3 Lo = vec3(0);
	uint maxRecursionDepth = uint(round(rnd(prd.randomSeed)*4+1));
	uint sampleCount = 1;
	for(uint i = 0; i < sampleCount; i++){
		// calculate per-light radiance
		vec3 L;

		vec3 radiance;

		bool isDiffuse = false;
		vec3 F    = fresnelSchlick(max(dot(N, V), 0.0), F0);       
		
		vec3 kS = F;
		vec3 kD = vec3(1.0) - kS;
		kD *= 1.0 - metallic;
		if(prd.recursionDepth < maxRecursionDepth){
			vec3 tangent, bitangent;
			createCoordinateSystem(worldNormal, tangent, bitangent);

			// when the surface has a diffuse part 50% chance to trace the diffuse part and 50% the specular
			if(rnd(prd.randomSeed)<(1-metallic)/2){
				isDiffuse = true;
				L = samplingHemisphere(prd.randomSeed, tangent, bitangent, N);
			}
			else{
				isDiffuse = false;
				L = reflect(-V, samplingFacetNormal(prd.randomSeed, tangent, bitangent, N, roughness));
			}


			uint  rayFlags          = gl_RayFlagsOpaqueEXT;
			float tMin              = 0.001;
			float tMax              = 10000.0;

			prd.rayOrigin             = worldPos;
			prd.rayDirection          = L;

			prd.recursionDepth++;
			traceRayEXT(
				accelerationStructure,    // acceleration structure
				rayFlags,                 // rayFlags
				0xFF,                     // cullMask
				0,                        // sbtRecordOffset
				0,                        // sbtRecordStride
				0,                        // missIndex
				prd.rayOrigin,            // ray origin
				tMin,                     // ray min range
				prd.rayDirection,         // ray direction
				tMax,                     // ray max range
				0                         // payload (location = 0)
			);
			prd.recursionDepth--;
			radiance = prd.radiance;
		}
		
		vec3 H = normalize(V + L);

		// cook-torrance brdf
		float NDF = DistributionGGX(N, H, roughness);
		float G   = GeometrySmith(N, V, L, roughness);
		
		vec3 numerator    = G * F;
		float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001;
		vec3 specular     = numerator / denominator;  

		// add to outgoing radiance Lo
		float NdotL = max(dot(N, L), 0.0);
		vec3 BRDF = vec3(0);
		if(isDiffuse){
			BRDF =  max(kD * albedo * (2-metallic) / PI, 0.0);// if the surface is not metallic multiply to correct for 50% chance to add up to 100% (diffuse*2 + specular*2)/2 == diffuse + specular
			NdotL = 1;
		} else
			BRDF = max(specular * (2-metallic), 0.0);
		Lo += emissive.xyz*emissive.w + BRDF * radiance * NdotL * 2;
	}

	prd.radiance  = Lo/sampleCount;
}