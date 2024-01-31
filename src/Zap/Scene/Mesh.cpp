#include "Zap/Scene/Mesh.h"

namespace Zap {
	std::vector<Mesh> Mesh::all;

	Mesh::Mesh(){
		m_id = all.size();
		all.push_back(*this);
		all.back().init();
	}

	Mesh::~Mesh() {}

	void Mesh::init() {
		if (m_isInit) return;
		m_isInit = true;
	}

	void Mesh::destroy() {
		if (!m_isInit) return;
		m_isInit = false;

		m_vertexBuffer.destroy();
		m_indexBuffer.destroy();
	}

	void Mesh::load(uint32_t vertexCount, Vertex* pVertices, uint32_t indexCount, uint32_t* pIndices) {
		m_vertexBuffer = vk::Buffer(vertexCount * sizeof(Vertex), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);
		m_vertexBuffer.init(); m_vertexBuffer.allocate(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
		m_vertexBuffer.uploadData(m_vertexBuffer.getSize(), pVertices);

		m_indexBuffer = vk::Buffer(indexCount * sizeof(uint32_t), VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);
		m_indexBuffer.init(); m_indexBuffer.allocate(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
		m_indexBuffer.uploadData(m_indexBuffer.getSize(), pIndices);
	}

	void Mesh::load(std::vector<Vertex> vertexArray, std::vector<uint32_t> indexArray) {
		load(vertexArray.size(), vertexArray.data(), indexArray.size(), indexArray.data());
	}

	uint32_t Mesh::getId() {
		return m_id;
	}

	vk::CommandBuffer* Mesh::getCommandBuffer(int index) {
		return &m_commandBuffers[index];
	}

	vk::Buffer* Mesh::getVertexBuffer() {
		return &m_vertexBuffer;
	}

	vk::Buffer* Mesh::getIndexbuffer() {
		return &m_indexBuffer;
	}

	Mesh* Mesh::createMesh() {
		return &Mesh::all[Mesh().m_id];
	}
}