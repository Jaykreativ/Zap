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
		std::cout << modelPath << " -> NumMeshes: " << scene->mNumMeshes << "\n";


		for (uint32_t i = 0; i < scene->mNumMeshes; i++) {
			meshIds.push_back(loadMesh(scene->mMeshes[i]));
		}

		vk::Buffer vertexStgBuffer = vk::Buffer(0, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
		vk::Buffer indexStgBuffer = vk::Buffer(0, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);

		for (int i = 0; i < scene->mNumMeshes; i++) {
			vertexStgBuffer.resize(vertexStgBuffer.getSize() + scene->mMeshes[i]->mNumVertices * sizeof(Vertex));
			indexStgBuffer.resize(indexStgBuffer.getSize() + scene->mMeshes[i]->mNumFaces * 3 * sizeof(uint32_t));
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
				vertexData[j] = Vertex({ pos.x, pos.y, pos.z }, { normal.x, normal.y, normal.z, });
			}
			vertexData += mesh->mNumVertices;

			uint32_t highestIndexTmp = highestIndex;
			for (uint32_t j = 0; j < mesh->mNumFaces; j++) {
				for (uint32_t k = 0; k < 3; k++) {
					indexData[j * 3 + k] = mesh->mFaces[j].mIndices[k] + highestIndexTmp;
					if (indexData[j * 3 + k] + 1 > highestIndex) highestIndex = indexData[j * 3 + k] + 1;
				}
			}
			indexData += mesh->mNumFaces * 3;
		}
		vertexStgBuffer.unmap();
		indexStgBuffer.unmap();

		Zap::Mesh* mesh = &Zap::Mesh::all[Zap::Mesh().getId()];

		mesh->m_vertexBuffer = vk::Buffer(vertexStgBuffer.getSize(), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);
		mesh->m_vertexBuffer.init(); mesh->m_vertexBuffer.allocate(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
		mesh->m_vertexBuffer.uploadData(&vertexStgBuffer);

		mesh->m_indexBuffer = vk::Buffer(indexStgBuffer.getSize(), VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);
		mesh->m_indexBuffer.init(); mesh->m_indexBuffer.allocate(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
		mesh->m_indexBuffer.uploadData(&indexStgBuffer);

		vertexStgBuffer.destroy();
		indexStgBuffer.destroy();

		std::cout << "mesh->m_vertexBuffer: " << mesh->m_vertexBuffer << "\n";
		std::cout << "mesh->m_indexBuffer: " << mesh->m_indexBuffer << "\n";

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

		mesh->m_vertexBuffer = vk::Buffer(vertexStgBuffer.getSize(), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);
		mesh->m_vertexBuffer.init(); mesh->m_vertexBuffer.allocate(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
		mesh->m_vertexBuffer.uploadData(&vertexStgBuffer);

		mesh->m_indexBuffer = vk::Buffer(indexStgBuffer.getSize(), VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);
		mesh->m_indexBuffer.init(); mesh->m_indexBuffer.allocate(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
		mesh->m_indexBuffer.uploadData(&indexStgBuffer);

		vertexStgBuffer.destroy();
		indexStgBuffer.destroy();

		return mesh->m_id;
	}
}