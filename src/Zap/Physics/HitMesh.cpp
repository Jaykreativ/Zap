#include "Zap/Physics/HitMesh.h"

#include "Zap/Zap.h"

namespace Zap {
	HitMesh::HitMesh()
		: m_handle()
	{
		Base::getBase()->m_assetHandler.m_hitMeshes[m_handle] = HitMeshData{};
	}

	HitMesh::HitMesh(UUID handle)
		: m_handle(handle)
	{}

	HitMesh::~HitMesh() {}

	void HitMesh::destroy() {
		auto* base = Base::getBase();
		HitMeshData* data = base->m_assetHandler.getHitMeshDataPtr(m_handle);
		destroy(data);
	}

	void HitMesh::destroy(HitMeshData* data) {
		delete[] data->m_vertices;
		delete[] data->m_indices;
	}

	bool HitMesh::exists() const {
		auto* base = Base::getBase();
		return base->getAssetHandler()->existsHitMeshData(m_handle);
	}

	physx::PxConvexMeshDesc HitMesh::getConvexDesc() {
		auto* base = Base::getBase();
		HitMeshData* data = base->m_assetHandler.getHitMeshDataPtr(m_handle);

		physx::PxConvexMeshDesc convexDesc;
		convexDesc.points.count = data->m_indexCount;
		convexDesc.points.stride = sizeof(glm::vec3);
		convexDesc.points.data = data->m_indices;
		convexDesc.flags = physx::PxConvexFlag::eCOMPUTE_CONVEX;

		return convexDesc;
	}

	UUID HitMesh::getHandle() {
		return m_handle;
	}

}