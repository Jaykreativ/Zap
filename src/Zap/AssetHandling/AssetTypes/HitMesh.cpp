#include "Zap/AssetHandling/AssetTypes/HitMesh.h"

#include "Zap/Zap.h"

namespace Zap {
	HitMesh::HitMesh(
		size_t     vertexCount,
		glm::vec3* vertices,
		size_t     indexCount,
		uint32_t*  indices
	) 
		:
		m_vertexCount(vertexCount),
		m_vertices(vertices),
		m_indexCount(indexCount),
		m_indices(indices)
	{}

	HitMesh::~HitMesh() {
		delete[] m_vertices;
		delete[] m_indices;
	}

	physx::PxConvexMeshDesc HitMesh::getConvexDesc() {
		auto* base = Base::getBase();
		physx::PxConvexMeshDesc convexDesc;
		convexDesc.points.count = m_vertexCount;
		convexDesc.points.stride = sizeof(glm::vec3);
		convexDesc.points.data = m_vertices;
		convexDesc.flags = physx::PxConvexFlag::eCOMPUTE_CONVEX;

		return convexDesc;
	}
}