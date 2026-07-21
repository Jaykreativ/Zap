#pragma once

#include "Zap/UUID.h"
#include "Zap/AssetHandling/Asset.h"
#include "Zap/Vertex.h"

#include "VulkanFramework.h"

#define PX_PHYSX_STATIC_LIB
#include "PxPhysicsAPI.h"

namespace Zap {
	class Mesh : public Asset {
	public:
		Mesh(
			size_t     pointCount,
			glm::vec3* points,
			size_t     indexCount,
			uint32_t*  indices,
			vk::Buffer vertexBuffer,
			vk::Buffer indexBuffer,
			glm::vec3  boundMax,
			glm::vec3  boundMin
		);
		~Mesh();

		const glm::vec3& getBoundMin() const;
		const glm::vec3& getBoundMax() const;

		const vk::Buffer& getVertexBuffer() const;

		const vk::Buffer& getIndexBuffer() const;

		physx::PxConvexMeshDesc getPxConvexMeshDesc() const;

	private:
		size_t     m_pointCount;
		glm::vec3* m_points;
		size_t     m_indexCount;
		uint32_t*  m_indices;

		glm::vec3 m_boundMin = { 0, 0, 0 };
		glm::vec3 m_boundMax = { 0, 0, 0 };

		vk::Buffer m_vertexBuffer = vk::Buffer();
		vk::Buffer m_indexBuffer = vk::Buffer();
	};
}

