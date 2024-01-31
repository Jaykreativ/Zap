#include "Zap/Scene/Mesh.h"
#include "Zap/ModelLoader.h"
#include "Zap/Vertex.h"
#include "assimp/Importer.hpp"
#include "assimp/scene.h"
#include "assimp/postprocess.h"

namespace Zap {
	ModelLoader::ModelLoader() {

	}

	ModelLoader::~ModelLoader() {

	}

	std::vector<uint32_t> ModelLoader::load(const char* modelPath) {
		std::vector<uint32_t> meshIds;

		Assimp::Importer importer;
		const aiScene* scene = importer.ReadFile(modelPath, aiProcess_Triangulate);

#ifdef _DEBUG
		std::cout << modelPath << " -> NumMeshes: " << scene->mNumMeshes << "\n";
#endif

		for (uint32_t i = 0; i < scene->mNumMeshes; i++) {
			meshIds.push_back(loadMesh(scene->mMeshes[i]));
		}

		return meshIds;
	}

	uint32_t ModelLoader::loadMesh(aiMesh* aMesh) {
		vk::Buffer vertexStgBuffer = vk::Buffer(aMesh->mNumVertices * sizeof(Vertex), VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
		vk::Buffer indexStgBuffer = vk::Buffer(aMesh->mNumFaces*3 * sizeof(uint32_t), VK_BUFFER_USAGE_TRANSFER_SRC_BIT);

		vertexStgBuffer.init(); vertexStgBuffer.allocate(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
		indexStgBuffer.init(); indexStgBuffer.allocate(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

		{
			void* rawData;
			vertexStgBuffer.map(&rawData);
			Vertex* data = (Vertex*)rawData;
			for (uint32_t i = 0; i < aMesh->mNumVertices; i++) {
				data[i].m_pos = *((glm::vec3*)&aMesh->mVertices[i]);
				data[i].m_normal = *((glm::vec3*)&aMesh->mNormals[i]);
			}
			vertexStgBuffer.unmap();
		}

		{
			void* rawData;
			indexStgBuffer.map(&rawData);
			uint32_t* data = (uint32_t*)rawData;
			for (uint32_t i = 0; i < aMesh->mNumFaces; i++) {
				memcpy(data + 3 * i, aMesh->mFaces[i].mIndices, 3*sizeof(uint32_t));
			}
			indexStgBuffer.unmap();
		}

		Mesh* mesh = Mesh::createMesh();

		mesh->m_vertexBuffer = vk::Buffer(vertexStgBuffer.getSize(), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT
			| VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR
		);
		mesh->m_vertexBuffer.init(); mesh->m_vertexBuffer.allocate(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
		mesh->m_vertexBuffer.uploadData(&vertexStgBuffer);

		mesh->m_indexBuffer = vk::Buffer(indexStgBuffer.getSize(), VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT
			| VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR
		);
		mesh->m_indexBuffer.init(); mesh->m_indexBuffer.allocate(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
		mesh->m_indexBuffer.uploadData(&indexStgBuffer);

		vertexStgBuffer.destroy();
		indexStgBuffer.destroy();

		return mesh->m_id;
	}
}