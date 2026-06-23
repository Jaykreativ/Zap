#pragma once

#include "Zap/UUID.h"

#define PX_PHYSX_STATIC_LIB
#include "PxPhysicsAPI.h"

#include "glm.hpp"

namespace Zap {
	class HitMesh {
		friend class HitMeshLoader;
	public:
		HitMesh();
		~HitMesh();

		physx::PxConvexMeshDesc getConvexDesc();

	private:
		size_t m_vertexCount;
		glm::vec3* m_vertices;
		size_t m_indexCount;
		uint32_t* m_indices;
	};
}