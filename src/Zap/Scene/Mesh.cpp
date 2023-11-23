#include "Zap/Scene/Mesh.h"

namespace Zap {
	std::vector<Mesh> Mesh::all;

	Mesh::Mesh(){
		m_id = all.size();
		all.push_back(*this);
		all.back().init();
	}

	Mesh::~Mesh() {
		if (!m_isInit) return;
		m_isInit = false;

		m_vertexBuffer.~Buffer();
		m_indexBuffer.~Buffer();
	}

	void Mesh::init() {
		if (m_isInit) return;
		m_isInit = true;
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

	void Mesh::load(const char* modelPath) {
		Assimp::Importer importer;
		const aiScene* scene = importer.ReadFile(modelPath, aiProcess_Triangulate);
		std::cout << modelPath << " -> NumMeshes: " << scene->mNumMeshes << "\n";

		vk::Buffer vertexStgBuffer = vk::Buffer(0, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
		vk::Buffer indexStgBuffer = vk::Buffer(0, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);

		for (int i = 0; i < scene->mNumMeshes; i++) {
			vertexStgBuffer.resize(vertexStgBuffer.getSize() + scene->mMeshes[i]->mNumVertices*sizeof(Vertex));
			indexStgBuffer.resize(indexStgBuffer.getSize() + scene->mMeshes[i]->mNumFaces*3*sizeof(uint32_t));
		}

		vertexStgBuffer.init(); vertexStgBuffer.allocate(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
		indexStgBuffer.init(); indexStgBuffer.allocate(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

		void* rawVertexData; vertexStgBuffer.map(&rawVertexData);
		void* rawIndexData; indexStgBuffer.map(&rawIndexData);
		Vertex* vertexData = (Vertex*)(rawVertexData);
		uint32_t* indexData = (uint32_t*)(rawIndexData);
		uint32_t highestIndex = 0;
		for (uint32_t i = 0; i < scene->mNumMeshes; i++) {
			aiMesh* mesh = scene->mMeshes[i];
			//memcpy(vertexData, mesh->mVertices, mesh->mNumVertices * sizeof(Vertex));
			for (uint32_t j = 0; j < mesh->mNumVertices; j++) {
				aiVector3D pos = mesh->mVertices[j];
				aiVector3D normal = mesh->mNormals[j];
				vertexData[j] = Vertex({ pos.x, pos.y, pos.z }, { normal.x, normal.y, normal.z,});
			}
			vertexData += mesh->mNumVertices;

			uint32_t highestIndexTmp = highestIndex;
			for (uint32_t j = 0; j < mesh->mNumFaces; j++) {
				for (uint32_t k = 0; k < 3; k++) { 
					indexData[j * 3 + k] = mesh->mFaces[j].mIndices[k] + highestIndexTmp;
					if (indexData[j * 3 + k]+1 > highestIndex) highestIndex = indexData[j * 3 + k]+1;
				}
			}
			indexData += mesh->mNumFaces*3;
		}
		vertexStgBuffer.unmap();
		indexStgBuffer.unmap();

		m_vertexBuffer = vk::Buffer(vertexStgBuffer.getSize(), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);
		m_vertexBuffer.init(); m_vertexBuffer.allocate(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
		m_vertexBuffer.uploadData(&vertexStgBuffer);

		m_indexBuffer = vk::Buffer(indexStgBuffer.getSize(), VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);
		m_indexBuffer.init(); m_indexBuffer.allocate(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
		m_indexBuffer.uploadData(&indexStgBuffer);

		vertexStgBuffer.~Buffer();
		indexStgBuffer.~Buffer();
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
}