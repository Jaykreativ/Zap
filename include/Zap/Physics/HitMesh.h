#pragma once

#include "Zap/Zap.h"
#include "Zap/UUID.h"

#include "glm.hpp"

namespace Zap {
	struct HitMeshData {
		size_t m_vertexCount;
		glm::vec3* m_vertices;
		size_t m_indexCount;
		uint32_t* m_indices;
	};

	class HitMesh {
	public:
		HitMesh();
		HitMesh(UUID handle);
		~HitMesh();

		void destroy();
		static void destroy(HitMeshData* data);

		bool exists() const;

		physx::PxConvexMeshDesc getConvexDesc();

		UUID getHandle();

	private:
		UUID m_handle;
	};
}