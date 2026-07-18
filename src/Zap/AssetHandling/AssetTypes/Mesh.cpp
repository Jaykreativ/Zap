#include "Zap/AssetHandling/AssetTypes/Mesh.h"

#include "Zap/Zap.h"

namespace Zap {
	Mesh::Mesh(
		vk::Buffer vertexBuffer,
		vk::Buffer indexBuffer,
		glm::vec3  boundMax,
		glm::vec3  boundMin
	)
		:
		m_vertexBuffer(vertexBuffer),
		m_indexBuffer(indexBuffer),
		m_boundMax(boundMax),
		m_boundMin(boundMin)
	{}

	Mesh::~Mesh() {
		m_indexBuffer.destroy();
		m_vertexBuffer.destroy();
	}

	//void Mesh::load(uint32_t vertexCount, Vertex* pVertices, uint32_t indexCount, uint32_t* pIndices) {
	//	auto* base = Base::getBase();
	//	MeshData* data = base->m_assetHandler.getMeshDataPtr(m_handle);
	//	data->m_vertexBuffer = vk::Buffer(vertexCount * sizeof(Vertex), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);
	//	data->m_vertexBuffer.init(); 
	//	data->m_vertexBuffer.allocate(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	//	data->m_vertexBuffer.uploadData(data->m_vertexBuffer.getSize(), pVertices);
	//	
	//	data->m_indexBuffer = vk::Buffer(indexCount * sizeof(uint32_t), VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);
	//	data->m_indexBuffer.init(); 
	//	data->m_indexBuffer.allocate(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	//	data->m_indexBuffer.uploadData(data->m_indexBuffer.getSize(), pIndices);
	//}

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
}