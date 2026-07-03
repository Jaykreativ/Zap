#pragma once

#include "Zap/UUID.h"
#include "Zap/AssetHandling/Asset.h"

#define PX_PHYSX_STATIC_LIB
#include "PxPhysicsAPI.h"

#include "glm.hpp"

namespace Zap {
	class HitMesh : public Asset {
	public:
		HitMesh(
			size_t     vertexCount,
			glm::vec3* vertices,
			size_t     indexCount,
			uint32_t*  indices
		);
		~HitMesh();

		physx::PxConvexMeshDesc getConvexDesc();

	private:
		size_t m_vertexCount;
		glm::vec3* m_vertices;
		size_t m_indexCount;
		uint32_t* m_indices;
	};
}