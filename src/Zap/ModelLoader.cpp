#include "Zap/Scene/Mesh.h"
#include "Zap/Scene/Model.h"
#include "Zap/ModelLoader.h"
#include "Zap/Vertex.h"
#include "Zap/Rendering/stb_image.h"
#include "assimp/Importer.hpp"
#include "assimp/scene.h"
#include "assimp/postprocess.h"

namespace Zap {
	ModelLoader::ModelLoader() {

	}

	ModelLoader::~ModelLoader() {

	}

	Model ModelLoader::load(const char* modelPath, uint32_t flags) { // TODO add support for .obj materials
		std::vector<Material> materials;
		std::vector<uint32_t> meshIds;

		Assimp::Importer importer;
		const aiScene* scene = importer.ReadFile(modelPath, aiProcess_Triangulate | aiProcess_GenUVCoords);

		auto pathSeperate = std::string(modelPath);
		pathSeperate.erase(pathSeperate.find_last_of("/")+1);

#ifdef _DEBUG
		std::cout << modelPath << " -> NumMeshes: " << scene->mNumMeshes << "\n";
#endif

		for (uint32_t i = 0; i < scene->mNumMeshes; i++) {
			meshIds.push_back(loadMesh(scene->mMeshes[i]));
			auto aMaterial = scene->mMaterials[scene->mMeshes[i]->mMaterialIndex];
			Material material = Material();
			aiColor4D aDiffuse; aiGetMaterialColor(aMaterial, AI_MATKEY_COLOR_DIFFUSE, &aDiffuse);
			material.albedoColor = glm::vec3(aDiffuse.r, aDiffuse.g, aDiffuse.b);
			aiGetMaterialFloat(aMaterial, AI_MATKEY_METALLIC_FACTOR, &material.metallic);
			aiGetMaterialFloat(aMaterial, AI_MATKEY_ROUGHNESS_FACTOR, &material.roughness);
			if (aiGetMaterialTextureCount(aMaterial, aiTextureType_DIFFUSE) > 0) {
				aiString diffuseTexturePath; aiGetMaterialTexture(aMaterial, aiTextureType_DIFFUSE, 0, &diffuseTexturePath);
				auto embeddedTexture = scene->GetEmbeddedTexture(diffuseTexturePath.C_Str());
				if (embeddedTexture) {
					material.albedoMap = loadTexture(embeddedTexture);
				}
				else {
					material.albedoMap = loadTexture((pathSeperate + std::string(diffuseTexturePath.C_Str())).c_str());
				}
				if (!ZP_IS_FLAG_ENABLED(flags, eTintTextures))
					material.albedoColor = glm::vec3(1, 1, 1);
			}
			if (aiGetMaterialTextureCount(aMaterial, aiTextureType_METALNESS) > 0) {
				aiString metallicTexturePath; aiGetMaterialTexture(aMaterial, aiTextureType_METALNESS, 0, &metallicTexturePath);
				auto embeddedTexture = scene->GetEmbeddedTexture(metallicTexturePath.C_Str());
				if (embeddedTexture) {
					material.metallicMap = loadTexture(embeddedTexture);
				}
				else {
					material.metallicMap = loadTexture((pathSeperate + std::string(metallicTexturePath.C_Str())).c_str());
				}
				if (!ZP_IS_FLAG_ENABLED(flags, eTintTextures))
					material.metallic = 1;
			}
			if (aiGetMaterialTextureCount(aMaterial, aiTextureType_DIFFUSE_ROUGHNESS) > 0) {
				aiString roughnessTexturePath; aiGetMaterialTexture(aMaterial, aiTextureType_DIFFUSE_ROUGHNESS, 0, &roughnessTexturePath);
				auto embeddedTexture = scene->GetEmbeddedTexture(roughnessTexturePath.C_Str());
				if (embeddedTexture) {
					material.roughnessMap = loadTexture(embeddedTexture);
				}
				else {
					material.roughnessMap = loadTexture((pathSeperate + std::string(roughnessTexturePath.C_Str())).c_str());
				}
				if (!ZP_IS_FLAG_ENABLED(flags, eTintTextures))
					material.roughness = 1;
			}
			materials.push_back(material);
		}

		Model model = { materials, meshIds };

		return model;
	}

	uint32_t ModelLoader::loadTexture(void* data, uint32_t width, uint32_t height) {
		auto base = Base::getBase();
		base->m_textures.push_back(vk::Image());
		vk::Image* image = &base->m_textures.back();
		image->setAspect(VK_IMAGE_ASPECT_COLOR_BIT);
		image->setExtent({ width, height, 1 });
		image->setFormat(VK_FORMAT_R8G8B8A8_UNORM); // TODO look for 1cmp formats
		image->setUsage(VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT);
		image->setType(VK_IMAGE_TYPE_2D);

		image->init();
		image->allocate(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
		image->initView();

		image->changeLayout(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_ACCESS_TRANSFER_WRITE_BIT);

		image->uploadData(width * height * 4, data);

		image->changeLayout(VK_IMAGE_LAYOUT_GENERAL, VK_ACCESS_SHADER_READ_BIT);

		return base->m_textures.size() - 1;
	}

	uint32_t ModelLoader::loadTexture(const char* texturePath) {
		int width, height, channels;
		stbi_set_flip_vertically_on_load(true);
		auto data = stbi_load(texturePath, &width, &height, &channels, 4);
		ZP_ASSERT(data, "Image not loaded correctly");
		return loadTexture(data, width, height);
	}

	uint32_t ModelLoader::loadTexture(const aiTexture* texture) {
		ZP_ASSERT(!texture->mHeight, "Raw texture loading not implemented Yet");// TODO implement raw texture loading
		int width, height, channels;
		stbi_set_flip_vertically_on_load(true);
		auto data = stbi_load_from_memory((stbi_uc*)texture->pcData, texture->mWidth, &width, &height, &channels, 4);
		ZP_ASSERT(data, "Image not loaded correctly");
		return loadTexture(data, width, height);
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
				data[i].pos = *((glm::vec3*)&aMesh->mVertices[i]);
				data[i].texCoords = *((glm::vec2*)&aMesh->mTextureCoords[0][i]);
				data[i].normal = *((glm::vec3*)&aMesh->mNormals[i]);
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