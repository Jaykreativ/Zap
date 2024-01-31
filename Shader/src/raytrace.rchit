#version 460
#extension GL_EXT_ray_tracing : require

#extension GL_EXT_shader_explicit_arithmetic_types_int64 : require
#extension GL_EXT_buffer_reference2 : require

#define PI (3.14159265359)

hitAttributeEXT vec2 attribs;

layout(location = 0) rayPayloadInEXT vec3 prd;

const int vertexSize = 6; // in floats
struct Vertex {
	vec3 pos;
	vec3 normal;
};

struct PerMeshData {
	uint64_t vertexAddress;
	uint64_t indexAddress;
};

layout(buffer_reference) buffer Vertices { float v[]; };
layout(buffer_reference) buffer Indices { uint i[]; };
layout(set = 0, binding = 2) buffer PerMesh { PerMeshData i[]; } perMesh;

struct LightData {
	vec3 pos;
	vec3 color;
};

layout(set=1, binding=1) readonly buffer LightBuffer {
	LightData l[];
} lights;

layout(set=1, binding = 0) uniform CamUBO{
	mat4 inverseView;
	mat4 inversePerspective;
	uint lightCount;
} camUBO;

vec3 lambertian(LightData light, vec3 pos, vec3 normal){
		vec3 vL = light.pos - pos;
		vec3 n = normal;
		vec3 E = light.color/pow(length(vL), 2)*dot(n, vL);
		vec3 L = 1/PI*E;
		if(L.x<0) L.x=0;
		if(L.y<0) L.y=0;
		if(L.z<0) L.z=0;
		return L;
}

void main() {
	PerMeshData meshData = perMesh.i[gl_InstanceCustomIndexEXT];
	Vertices vertices    = Vertices(meshData.vertexAddress);
	Indices indices      = Indices(meshData.indexAddress);

	uint ind0 = indices.i[gl_PrimitiveID*3+0];
	uint ind1 = indices.i[gl_PrimitiveID*3+1];
	uint ind2 = indices.i[gl_PrimitiveID*3+2];

	// Vertex of the triangle
	Vertex v0 = Vertex(
		vec3(vertices.v[ind0*vertexSize+0], vertices.v[ind0*vertexSize+1], vertices.v[ind0*vertexSize+2]),
		vec3(vertices.v[ind0*vertexSize+3], vertices.v[ind0*vertexSize+4], vertices.v[ind0*vertexSize+5])
	);
	Vertex v1 = Vertex(
		vec3(vertices.v[ind1*vertexSize+0], vertices.v[ind1*vertexSize+1], vertices.v[ind1*vertexSize+2]),
		vec3(vertices.v[ind1*vertexSize+3], vertices.v[ind1*vertexSize+4], vertices.v[ind1*vertexSize+5])
	);
	Vertex v2 = Vertex(
		vec3(vertices.v[ind2*vertexSize+0], vertices.v[ind2*vertexSize+1], vertices.v[ind2*vertexSize+2]),
		vec3(vertices.v[ind2*vertexSize+3], vertices.v[ind2*vertexSize+4], vertices.v[ind2*vertexSize+5])
	);
	
	const vec3 barycentrics = vec3(1.0 - attribs.x - attribs.y, attribs.x, attribs.y);
	
	// Computing the coordinates of the hit position
	const vec3 pos      = v0.pos * barycentrics.x + v1.pos * barycentrics.y + v2.pos * barycentrics.z;
	const vec3 worldPos = vec3(gl_ObjectToWorldEXT * vec4(pos, 1.0));  // Transforming the position to world space

	const vec3 normal      = v0.normal * barycentrics.x + v1.normal * barycentrics.y + v2.normal * barycentrics.z;
	const vec3 worldNormal = normalize(vec3(normal * gl_WorldToObjectEXT));  // Transforming the normal to world space

	vec3 light = vec3(0.1, 0.1, 0.1);
	for(uint i=0; i<camUBO.lightCount; i++){
		if(false) continue;
		light += lambertian(lights.l[i], worldPos, worldNormal);
	}
	prd = vec3(light);
}