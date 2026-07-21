#include "Zap/AssetHandling/AssetTypes/Mesh.h"

#include "Zap/Zap.h"

namespace Zap {
	Mesh::Mesh(
		size_t     pointCount,
		glm::vec3* points,
		size_t     indexCount,
		uint32_t*  indices,
		vk::Buffer vertexBuffer,
		vk::Buffer indexBuffer,
		glm::vec3  boundMax,
		glm::vec3  boundMin
	)
		:
		m_pointCount(pointCount),
		m_points(points),
		m_indexCount(indexCount),
		m_indices(indices),
		m_vertexBuffer(vertexBuffer),
		m_indexBuffer(indexBuffer),
		m_boundMax(boundMax),
		m_boundMin(boundMin)
	{}

	Mesh::~Mesh() {
		delete[] m_points;
		delete[] m_indices;
		m_indexBuffer.destroy();
		m_vertexBuffer.destroy();
	}

	const glm::vec3& Mesh::getBoundMin() const {
		return m_boundMin;
	}
	const glm::vec3& Mesh::getBoundMax() const {
		return m_boundMax;
	}

	const vk::Buffer& Mesh::getVertexBuffer() const {
		return m_vertexBuffer;
	}

	const vk::Buffer& Mesh::getIndexBuffer() const {
		return m_indexBuffer;
	}

	physx::PxConvexMeshDesc Mesh::getPxConvexMeshDesc() const {
		auto* base = Base::getBase();
		physx::PxConvexMeshDesc convexDesc;
		convexDesc.points.count = m_pointCount;
		convexDesc.points.stride = sizeof(glm::vec3);
		convexDesc.points.data = m_points;
		convexDesc.flags = physx::PxConvexFlag::eCOMPUTE_CONVEX;

		return convexDesc;
	}

}