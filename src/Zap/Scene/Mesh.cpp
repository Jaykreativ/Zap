#include "Zap/Scene/Mesh.h"

#include "Zap/Zap.h"

namespace Zap {
	Mesh::Mesh()
		: m_handle()
	{
		Base::getBase()->m_assetHandler.m_meshes[m_handle] = MeshData{};
	}

	Mesh::Mesh(UUID handle)
		: m_handle(handle)
	{}

	Mesh::~Mesh() {}

	void Mesh::load(uint32_t vertexCount, Vertex* pVertices, uint32_t indexCount, uint32_t* pIndices) {
		auto* base = Base::getBase();
		MeshData* data = base->m_assetHandler.getMeshDataPtr(m_handle);
		data->m_vertexBuffer = vk::Buffer(vertexCount * sizeof(Vertex), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);
		data->m_vertexBuffer.init(); 
		data->m_vertexBuffer.allocate(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
		data->m_vertexBuffer.uploadData(data->m_vertexBuffer.getSize(), pVertices);
		
		data->m_indexBuffer = vk::Buffer(indexCount * sizeof(uint32_t), VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);
		data->m_indexBuffer.init(); 
		data->m_indexBuffer.allocate(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
		data->m_indexBuffer.uploadData(data->m_indexBuffer.getSize(), pIndices);
	}

	void Mesh::load(std::vector<Vertex> vertexArray, std::vector<uint32_t> indexArray) {
		load(vertexArray.size(), vertexArray.data(), indexArray.size(), indexArray.data());
	}

	void Mesh::destroy() {
		auto* base = Base::getBase();
		MeshData* data = base->m_assetHandler.getMeshDataPtr(m_handle);
		destroy(data);
	}
	void Mesh::destroy(MeshData* data) {
		data->m_vertexBuffer.destroy();
		data->m_indexBuffer.destroy();
	}

	void Mesh::remove() {
		destroy();
		auto* base = Base::getBase();
		base->m_assetHandler.m_meshes.erase(m_handle);
	}

	void Mesh::setBoundingBox(glm::vec3 boundMin, glm::vec3 boundMax) {
		auto* base = Base::getBase();
		MeshData* data = base->m_assetHandler.getMeshDataPtr(m_handle);
		data->m_boundMin = boundMin;
		data->m_boundMax = boundMax;
	}

	bool Mesh::exists() const {
		auto* base = Base::getBase();
		return base->getAssetHandler()->existsMeshData(m_handle);
	}

	UUID Mesh::getHandle() {
		return m_handle;
	}

	const glm::mat4* Mesh::getTransform() const {
		auto* base = Base::getBase();
		auto& data = base->getAssetHandler()->getMeshData(m_handle);
		return &data.m_transform;
	}

	const vk::Buffer* Mesh::getVertexBuffer() const {
		auto* base = Base::getBase();
		auto& data = base->getAssetHandler()->getMeshData(m_handle);
		return &data.m_vertexBuffer;
	}

	const vk::Buffer* Mesh::getIndexBuffer() const {
		auto* base = Base::getBase();
		auto& data = base->getAssetHandler()->getMeshData(m_handle);
		return &data.m_indexBuffer;
	}
}