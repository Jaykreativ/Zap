#include "Zap/AssetHandling/AssetTypes/HitMesh.h"

#include "Zap/Zap.h"

namespace Zap {
	HitMesh::HitMesh() {}
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