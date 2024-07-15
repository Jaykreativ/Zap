#include "Zap/ModelLoader.h"

#include "Zap/Scene/Mesh.h"
#include "Zap/Scene/Material.h"
#include "Zap/Scene/Model.h"
#include "Zap/ModelLoader.h"
#include "Zap/Vertex.h"
#include "Zap/Rendering/stb_image.h"

glm::mat4 ConvertMatrixToGLMFormat(const aiMatrix4x4& from) {
	glm::mat4 to;
	//the a,b,c,d in assimp is the row ; the 1,2,3,4 is the column
	to[0][0] = from.a1; to[1][0] = from.a2; to[2][0] = from.a3; to[3][0] = from.a4;
	to[0][1] = from.b1; to[1][1] = from.b2; to[2][1] = from.b3; to[3][1] = from.b4;
	to[0][2] = from.c1; to[1][2] = from.c2; to[2][2] = from.c3; to[3][2] = from.c4;
	to[0][3] = from.d1; to[1][3] = from.d2; to[2][3] = from.d3; to[3][3] = from.d4;
	return to;
}

glm::vec3 GetGLMVec(const aiVector3D& vec) {
	return glm::vec3(vec.x, vec.y, vec.z);
}

//glm::quat GetGLMQuat(const aiQuaternion& pOrientation) {
//	return glm::quat(pOrientation.w, pOrientation.x, pOrientation.y, pOrientation.z);
//}

namespace Zap {
	ModelLoader::ModelLoader() {

	}

	ModelLoader::~ModelLoader() {

	}

	Model ModelLoader::load(const char* modelPath, uint32_t flags) { // TODO add support for better .obj materials
		Model model = {};
		model.filepath = std::string(modelPath);

		Assimp::Importer importer;
		const aiScene* aScene = importer.ReadFile(modelPath, aiProcess_Triangulate | aiProcess_GenUVCoords);

		auto pathSeperate = std::string(modelPath);
		pathSeperate.erase(pathSeperate.find_last_of("/")+1);

		ZP_ASSERT(aScene, (std::string("Scene can't be loaded, check the filepath: ") + std::string(modelPath)).c_str());

#ifdef _DEBUG
		std::cout << modelPath << " -> NumMeshes: " << aScene->mNumMeshes << "\n";
		auto timeStartLoad = std::chrono::high_resolution_clock::now();
#endif
		processNode(aScene->mRootNode, aScene, pathSeperate, glm::mat4(1), flags, &model);

#ifdef _DEBUG
		auto timeLoad = std::chrono::high_resolution_clock::now() - timeStartLoad;
		std::cout << modelPath << " -> Duration: " <<
			std::chrono::duration_cast<std::chrono::duration<float>>(timeLoad).count() << "s\n";
#endif
		return model;
	}

	void ModelLoader::processNode(const aiNode* node, const aiScene* aScene, std::string path, glm::mat4& transform, uint32_t flags, Model* pModel) {
		glm::mat4 newTransform = ConvertMatrixToGLMFormat(node->mTransformation) * transform;
		for (uint32_t i = 0; i < node->mNumMeshes; i++) {
#ifdef _DEBUG
			auto timeStartLoad = std::chrono::high_resolution_clock::now();
			auto timeStartMeshLoad = std::chrono::high_resolution_clock::now();
#endif

			pModel->meshes.push_back(loadMesh(aScene->mMeshes[node->mMeshes[i]], newTransform));

#ifdef _DEBUG
			auto timeMeshLoad = std::chrono::high_resolution_clock::now() - timeStartMeshLoad;
			auto timeStartMaterialLoad = std::chrono::high_resolution_clock::now();
#endif

			aiMaterial* aMaterial = aScene->mMaterials[aScene->mMeshes[node->mMeshes[i]]->mMaterialIndex];
			pModel->materials.push_back(Material());
			loadMaterial(aScene, aMaterial, path, flags, &pModel->materials.back());

#ifdef _DEBUG
			auto timeMaterialLoad = std::chrono::high_resolution_clock::now() - timeStartMaterialLoad;
			auto timeLoad = std::chrono::high_resolution_clock::now() - timeStartLoad;
			std::cout <<
				std::chrono::duration_cast<std::chrono::duration<float>>(timeLoad).count() * 1000 << "ms - " <<
				std::chrono::duration_cast<std::chrono::duration<float>>(timeMeshLoad).count() * 1000 << "ms(Mesh only) - " <<
				std::chrono::duration_cast<std::chrono::duration<float>>(timeMaterialLoad).count() * 1000 << "ms(Material only)\n";
#endif
		}

		for (uint32_t i = 0; i < node->mNumChildren; i++) {
			processNode(node->mChildren[i], aScene, path, newTransform, flags, pModel);
		}
	}

	Model ModelLoader::loadFromMemory(uint32_t vertexCount, Vertex* pVertices, uint32_t indexCount, uint32_t* pIndices) {
		Mesh mesh = Mesh();
		mesh.load(vertexCount, pVertices, indexCount, pIndices);
		return {"", {Material()}, {mesh}};
	}

	Model ModelLoader::loadFromMemory(std::vector<Vertex> vertexArray, std::vector<uint32_t> indexArray) {
		return loadFromMemory(vertexArray.size(), vertexArray.data(), indexArray.size(), indexArray.data());
	}

	void ModelLoader::loadMaterial(const aiScene* aScene, const aiMaterial* aMaterial, std::string path, uint32_t flags, Material* pMaterial) {;
		aiColor4D aDiffuse; aiGetMaterialColor(aMaterial, AI_MATKEY_COLOR_DIFFUSE, &aDiffuse);
		pMaterial->albedoColor = glm::vec3(aDiffuse.r, aDiffuse.g, aDiffuse.b);
		aiGetMaterialFloat(aMaterial, AI_MATKEY_METALLIC_FACTOR, &pMaterial->metallic);
		aiGetMaterialFloat(aMaterial, AI_MATKEY_ROUGHNESS_FACTOR, &pMaterial->roughness);
		aiColor4D aEmissive; aiGetMaterialColor(aMaterial, AI_MATKEY_COLOR_EMISSIVE, &aEmissive);
		pMaterial->emissive = glm::vec4(aEmissive.r, aEmissive.g, aEmissive.b, 0);
		aiGetMaterialFloat(aMaterial, AI_MATKEY_EMISSIVE_INTENSITY, &pMaterial->emissive.w);
		if (aiGetMaterialTextureCount(aMaterial, aiTextureType_DIFFUSE) > 0) {
			aiString diffuseTexturePath; aiGetMaterialTexture(aMaterial, aiTextureType_DIFFUSE, 0, &diffuseTexturePath);
			auto embeddedTexture = aScene->GetEmbeddedTexture(diffuseTexturePath.C_Str());
			if (embeddedTexture) {
				pMaterial->albedoMap = loadTexture(embeddedTexture);
			}
			else {
				pMaterial->albedoMap = loadTexture((path + std::string(diffuseTexturePath.C_Str())).c_str());
			}
			if (!ZP_IS_FLAG_ENABLED(flags, eTintTextures))
				pMaterial->albedoColor = glm::vec3(1, 1, 1);
		}
		if (aiGetMaterialTextureCount(aMaterial, aiTextureType_METALNESS) > 0) {
			aiString metallicTexturePath; aiGetMaterialTexture(aMaterial, aiTextureType_METALNESS, 0, &metallicTexturePath);
			auto embeddedTexture = aScene->GetEmbeddedTexture(metallicTexturePath.C_Str());
			if (embeddedTexture) {
				pMaterial->metallicMap = loadTexture(embeddedTexture);
			}
			else {
				pMaterial->metallicMap = loadTexture((path + std::string(metallicTexturePath.C_Str())).c_str());
			}
			if (!ZP_IS_FLAG_ENABLED(flags, eTintTextures))
				pMaterial->metallic = 1;
		}
		if (aiGetMaterialTextureCount(aMaterial, aiTextureType_DIFFUSE_ROUGHNESS) > 0) {
			aiString roughnessTexturePath; aiGetMaterialTexture(aMaterial, aiTextureType_DIFFUSE_ROUGHNESS, 0, &roughnessTexturePath);
			auto embeddedTexture = aScene->GetEmbeddedTexture(roughnessTexturePath.C_Str());
			if (embeddedTexture) {
				pMaterial->roughnessMap = loadTexture(embeddedTexture);
			}
			else {
				pMaterial->roughnessMap = loadTexture((path + std::string(roughnessTexturePath.C_Str())).c_str());
			}
			if (!ZP_IS_FLAG_ENABLED(flags, eTintTextures))
				pMaterial->roughness = 1;
		}
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
		Zap::Base::getBase()->m_texturePaths.push_back(texturePath);
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

	Mesh ModelLoader::loadMesh(aiMesh* aMesh, glm::mat4& transform) {
		Base* base = Base::getBase();

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

		Mesh mesh = Mesh();
		mesh.init();

		MeshData* pMeshData = base->m_assetHandler.getMeshDataPtr(mesh.getHandle());

		pMeshData->m_transform = transform;

		pMeshData->m_vertexBuffer = vk::Buffer(vertexStgBuffer.getSize(), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT
			| VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR
		);
		pMeshData->m_vertexBuffer.init(); pMeshData->m_vertexBuffer.allocate(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
		pMeshData->m_vertexBuffer.uploadData(&vertexStgBuffer);

		pMeshData->m_indexBuffer = vk::Buffer(indexStgBuffer.getSize(), VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT
			| VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR
		);
		pMeshData->m_indexBuffer.init(); pMeshData->m_indexBuffer.allocate(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
		pMeshData->m_indexBuffer.uploadData(&indexStgBuffer);

		vertexStgBuffer.destroy();
		indexStgBuffer.destroy();

		return mesh;
	}
}