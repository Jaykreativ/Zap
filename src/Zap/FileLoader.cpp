#include "Zap/FileLoader.h"

#include "Zap/Scene/Model.h"
#include "Zap/Scene/Mesh.h"
#include "Zap/Scene/Material.h"
#include "Zap/Scene/Camera.h"
#include "Zap/Scene/Light.h"
#include "Zap/Scene/PhysicsComponent.h"
#include "Zap/Scene/Transform.h"
#include "Zap/Scene/Texture.h"
#include "Zap/Scene/Actor.h"
#include "Zap/Scene/Scene.h"
#include "Zap/Serializer.h"
#include "Zap/Vertex.h"
#include "Zap/Rendering/stb_image.h"

#include <sstream>
#include <fstream>

namespace Zap {
	Image ImageLoader::load(void* data, uint32_t width, uint32_t height) {
		auto base = Base::getBase();
		Image image;
		image.setAspect(VK_IMAGE_ASPECT_COLOR_BIT);
		image.setExtent({ width, height, 1 });
		image.setFormat(VK_FORMAT_R8G8B8A8_UNORM); // TODO look for 1cmp formats
		image.setUsage(VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT);
		image.setType(VK_IMAGE_TYPE_2D);

		image.init();
		image.allocate(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
		image.initView();

		image.changeLayout(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_ACCESS_TRANSFER_WRITE_BIT);

		image.uploadData(width * height * 4, data);

		image.changeLayout(VK_IMAGE_LAYOUT_GENERAL, VK_ACCESS_SHADER_READ_BIT);

		return image;
	}

	Texture TextureLoader::load(std::string filepath, UUID handle) {
		auto& assetHandler = Base::getBase()->m_assetHandler;
		int width, height, channels;
		stbi_set_flip_vertically_on_load(true);
		auto data = stbi_load(filepath.c_str(), &width, &height, &channels, 4);
		ZP_ASSERT(data, "Image not loaded correctly");
		Texture texture = Texture(handle);
		assetHandler.m_textures[texture.getHandle()] = TextureData{};
		assetHandler.getTextureDataPtr(texture.getHandle())->image = ImageLoader::load(data, width, height);
		assetHandler.m_loadedTextures.push_back(texture);
		return texture;
	}

	Texture TextureLoader::load(const aiTexture* aiTexture, UUID handle) {
		ZP_ASSERT(!aiTexture->mHeight, "Raw texture loading not implemented Yet");// TODO implement raw texture loading
		auto& assetHandler = Base::getBase()->m_assetHandler;
		int width, height, channels;
		stbi_set_flip_vertically_on_load(true);
		auto data = stbi_load_from_memory((stbi_uc*)aiTexture->pcData, aiTexture->mWidth, &width, &height, &channels, 4);
		ZP_ASSERT(data, "Image not loaded correctly");
		Texture texture = Texture(handle);
		assetHandler.m_textures[texture.getHandle()] = TextureData{};
		assetHandler.getTextureDataPtr(texture.getHandle())->image = ImageLoader::load(data, width, height);
		assetHandler.m_loadedTextures.push_back(texture);
		return texture;
	}

	Material MaterialLoader::load(const aiScene* aScene, const aiMaterial* aMaterial, std::string path, UUID handle) {
		Material material = Material(handle);
		auto& assetHandler = Base::getBase()->m_assetHandler;
		assetHandler.m_materials[material.getHandle()] = MaterialData{};
		MaterialData* pMaterialData = assetHandler.getMaterialDataPtr(material.getHandle());

		aiColor4D aDiffuse; aiGetMaterialColor(aMaterial, AI_MATKEY_COLOR_DIFFUSE, &aDiffuse);
		pMaterialData->albedoColor = glm::vec4(aDiffuse.r, aDiffuse.g, aDiffuse.b, 1);
		aiGetMaterialFloat(aMaterial, AI_MATKEY_METALLIC_FACTOR, &pMaterialData->metallic);
		aiGetMaterialFloat(aMaterial, AI_MATKEY_ROUGHNESS_FACTOR, &pMaterialData->roughness);
		aiColor4D aEmissive; aiGetMaterialColor(aMaterial, AI_MATKEY_COLOR_EMISSIVE, &aEmissive);
		pMaterialData->emissive = glm::vec4(aEmissive.r, aEmissive.g, aEmissive.b, 0);
		aiGetMaterialFloat(aMaterial, AI_MATKEY_EMISSIVE_INTENSITY, &pMaterialData->emissive.w);
		if (aiGetMaterialTextureCount(aMaterial, aiTextureType_DIFFUSE) > 0) {
			aiString diffuseTexturePath; aiGetMaterialTexture(aMaterial, aiTextureType_DIFFUSE, 0, &diffuseTexturePath);
			auto embeddedTexture = aScene->GetEmbeddedTexture(diffuseTexturePath.C_Str());
			if (embeddedTexture) {
				pMaterialData->albedoMap = TextureLoader::load(embeddedTexture);
			}
			else {
				pMaterialData->albedoMap = TextureLoader::load((path + std::string(diffuseTexturePath.C_Str())).c_str());
			}
			if (!ZP_IS_FLAG_ENABLED(flags, eTintTextures))
				pMaterialData->albedoColor = glm::vec4(1, 1, 1, 1);
		}
		if (aiGetMaterialTextureCount(aMaterial, aiTextureType_METALNESS) > 0) {
			aiString metallicTexturePath; aiGetMaterialTexture(aMaterial, aiTextureType_METALNESS, 0, &metallicTexturePath);
			auto embeddedTexture = aScene->GetEmbeddedTexture(metallicTexturePath.C_Str());
			if (embeddedTexture) {
				pMaterialData->metallicMap = TextureLoader::load(embeddedTexture);
			}
			else {
				pMaterialData->metallicMap = TextureLoader::load((path + std::string(metallicTexturePath.C_Str())).c_str());
			}
			if (!ZP_IS_FLAG_ENABLED(flags, eTintTextures))
				pMaterialData->metallic = 1;
		}
		if (aiGetMaterialTextureCount(aMaterial, aiTextureType_DIFFUSE_ROUGHNESS) > 0) {
			aiString roughnessTexturePath; aiGetMaterialTexture(aMaterial, aiTextureType_DIFFUSE_ROUGHNESS, 0, &roughnessTexturePath);
			auto embeddedTexture = aScene->GetEmbeddedTexture(roughnessTexturePath.C_Str());
			if (embeddedTexture) {
				pMaterialData->roughnessMap = TextureLoader::load(embeddedTexture);
			}
			else {
				pMaterialData->roughnessMap = TextureLoader::load((path + std::string(roughnessTexturePath.C_Str())).c_str());
			}
			if (!ZP_IS_FLAG_ENABLED(flags, eTintTextures))
				pMaterialData->roughness = 1;
		}
		return material;
	}

	Mesh MeshLoader::load(aiMesh* aMesh, glm::mat4& transform, glm::vec3& modelBoundMin, glm::vec3& modelBoundMax, UUID handle) {
		Base* base = Base::getBase();
		auto& assetHandler = base->m_assetHandler;

		vk::Buffer vertexStgBuffer = vk::Buffer(aMesh->mNumVertices * sizeof(Vertex), VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
		vk::Buffer indexStgBuffer = vk::Buffer(aMesh->mNumFaces * 3 * sizeof(uint32_t), VK_BUFFER_USAGE_TRANSFER_SRC_BIT);

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
				memcpy(data + 3 * i, aMesh->mFaces[i].mIndices, 3 * sizeof(uint32_t));
			}
			indexStgBuffer.unmap();
		}

		Mesh mesh = Mesh(handle);
		assetHandler.m_meshes[mesh.getHandle()] = MeshData{};

		MeshData* pMeshData = assetHandler.getMeshDataPtr(mesh.getHandle());

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

		// Bounding box
		glm::vec3 boundMin = transform * glm::vec4(*((glm::vec3*)&aMesh->mAABB.mMin), 1);
		glm::vec3 boundMax = transform * glm::vec4(*((glm::vec3*)&aMesh->mAABB.mMax), 1);
		mesh.setBoundingBox(boundMin, boundMax);

		modelBoundMin = glm::min(modelBoundMin, boundMin);
		modelBoundMax = glm::max(modelBoundMax, boundMax);

		return mesh;
	}

	Mesh MeshLoader::loadFromFile(std::string filepath, uint32_t index, glm::mat4& transform, UUID handle) {
		if (Base::getBase()->m_assetHandler.m_pathMeshMap.count({ filepath, index })) {
			return Base::getBase()->m_assetHandler.m_pathMeshMap.at({ filepath, index });
		}
		Assimp::Importer importer;
		const aiScene* aScene = importer.ReadFile(filepath.c_str(), aiProcess_Triangulate | aiProcess_GenUVCoords | aiProcess_GenBoundingBoxes);

		ZP_ASSERT(aScene, (std::string("Scene can't be loaded, check the filepath: ") + std::string(filepath)).c_str());

		glm::vec3 boundMin, boundMax;
		auto mesh = load(aScene->mMeshes[index], transform, boundMin, boundMax, handle);
		Base::getBase()->m_assetHandler.m_loadedMeshes.push_back(mesh);
		Base::getBase()->m_assetHandler.m_meshPaths[mesh.getHandle()] = {filepath, index};
		Base::getBase()->m_assetHandler.m_pathMeshMap[{ filepath, index }] = mesh.getHandle();
		return mesh;
	}

	Model ModelLoader::load(std::string filepath) {
		Model model = {};
		model.filepath = std::string(filepath);

#ifdef _DEBUG
		auto timeStartLoad = std::chrono::high_resolution_clock::now();
#endif
		Assimp::Importer importer;
		const aiScene* aScene = importer.ReadFile(filepath.c_str(), aiProcess_Triangulate | aiProcess_GenUVCoords | aiProcess_GenBoundingBoxes);

		ZP_ASSERT(aScene, (std::string("Scene can't be loaded, check the filepath: ") + std::string(filepath)).c_str());

#ifdef _DEBUG
		std::cout << filepath << " -> NumMeshes: " << aScene->mNumMeshes << "\n";
#endif
		processNode(aScene->mRootNode, aScene, filepath, glm::mat4(1), model);

#ifdef _DEBUG
		auto timeLoad = std::chrono::high_resolution_clock::now() - timeStartLoad;
		std::cout << filepath << " -> Duration: " <<
			std::chrono::duration_cast<std::chrono::duration<float>>(timeLoad).count() << "s\n";
#endif
		return model;
	}

	void ModelLoader::processNode(const aiNode* node, const aiScene* aScene, std::string path, glm::mat4& transform, Model& model) {
		auto& assetHandler = Base::getBase()->m_assetHandler;

#ifdef _DEBUG
		std::cout << "Loading node " << node->mName.C_Str() << " with " << node->mNumChildren << " children\n";
#endif
		glm::mat4 newTransform = transform * AssimpUtils::mat4ToGlmMat4(node->mTransformation);
		for (uint32_t i = 0; i < node->mNumMeshes; i++) {
#ifdef _DEBUG
			auto timeStartLoad = std::chrono::high_resolution_clock::now();
			auto timeStartMeshLoad = std::chrono::high_resolution_clock::now();
#endif
			// Check if mesh is already loaded
			if (Base::getBase()->m_assetHandler.m_pathMeshMap.count({ path, node->mMeshes[i] })) {
				model.meshes.push_back(Base::getBase()->m_assetHandler.m_pathMeshMap.at({ path, node->mMeshes[i] }));
			}
			else {
				// Load mesh
				model.meshes.push_back(MeshLoader::load(aScene->mMeshes[node->mMeshes[i]], newTransform, model.boundMin, model.boundMax));
				assetHandler.m_loadedMeshes.push_back(model.meshes.back());
				Base::getBase()->m_assetHandler.m_meshPaths[model.meshes.back().getHandle()] = { path, node->mMeshes[i] };
				Base::getBase()->m_assetHandler.m_pathMeshMap[{ path, node->mMeshes[i] }] = model.meshes.back().getHandle();
			}

#ifdef _DEBUG
			auto timeMeshLoad = std::chrono::high_resolution_clock::now() - timeStartMeshLoad;
			auto timeStartMaterialLoad = std::chrono::high_resolution_clock::now();
#endif

			auto pathSeperate = std::string(path);
			pathSeperate.erase(pathSeperate.find_last_of("/") + 1);

			// Check if material already exists
			if (Base::getBase()->m_assetHandler.m_pathMaterialMap.count({ path, aScene->mMeshes[node->mMeshes[i]]->mMaterialIndex })) {
				model.materials.push_back(Base::getBase()->m_assetHandler.m_pathMaterialMap.at({ path, aScene->mMeshes[node->mMeshes[i]]->mMaterialIndex }));
			}
			else {
				// Load material
				aiMaterial* aMaterial = aScene->mMaterials[aScene->mMeshes[node->mMeshes[i]]->mMaterialIndex];
				model.materials.push_back(MaterialLoader::load(aScene, aMaterial, pathSeperate));
				assetHandler.m_loadedMaterials.push_back(model.materials.back());
				Base::getBase()->m_assetHandler.m_materialPaths[model.materials.back().getHandle()] = { path, aScene->mMeshes[node->mMeshes[i]]->mMaterialIndex };
				Base::getBase()->m_assetHandler.m_pathMaterialMap[{ path, aScene->mMeshes[node->mMeshes[i]]->mMaterialIndex }] = model.materials.back().getHandle();
			}

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
			processNode(node->mChildren[i], aScene, path, newTransform, model);
		}
	}

	Actor ActorLoader::load(std::string filepath, Scene* pScene) {
		Serializer serializer;
		Actor actor = Actor((UUID)0, pScene);
		if (serializer.beginDeserialization(filepath.c_str()) && serializer.beginElement("Actor")) {
			UUID handle = std::stoull(serializer.readAttribute("handle"));
			actor = Actor(handle, pScene);
			pScene->attachActor(actor);

			if (serializer.beginElement("Camera")) {
				glm::mat4 offset = serializer.readAttributeMat4("offset");
				actor.addCamera(offset);

				serializer.endElement();
			}
			if (serializer.beginElement("Light")) {
				glm::vec3 color = serializer.readAttributeVec3("color");;
				float strength = serializer.readAttributef("strength");
				float radius = serializer.readAttributef("radius");

				actor.addLight(color, strength, radius);
				
				serializer.endElement();
			}
			if (serializer.beginElement("Model")) {
				Model model;
				int meshCount = serializer.readAttributei("meshCount");
				for (size_t i = 0; i < meshCount; i++) {
					UUID handle = std::stoull(serializer.readAttribute("mesh" + std::to_string(i)));
					model.meshes.push_back(Mesh(handle));
				}

				int materialCount = serializer.readAttributei("materialCount");
				for (size_t i = 0; i < materialCount; i++) {
					UUID handle = std::stoull(serializer.readAttribute("material" + std::to_string(i)));
					model.materials.push_back(Material(handle));
				}

				actor.addModel(model);

				serializer.endElement();
			}
			if (serializer.beginElement("RigidDynamic")) {
				serializer.endElement();
			}
			if (serializer.beginElement("RigidStatic")) {
				serializer.endElement();
			}
			if (serializer.beginElement("Transform")) {
				glm::mat4 transform = serializer.readAttributeMat4("transform");
				actor.addTransform(transform);

				serializer.endElement();
			}

			serializer.endElement();
			serializer.endDeserialization();
		}
		return actor;
	}

	void ActorLoader::store(std::string filepath, Actor actor) {
		Serializer serializer;
		serializer.beginSerialization(filepath.c_str());
		serializer.beginElement("Actor");

		serializer.writeAttribute("handle", std::to_string(actor.getHandle()));

		if (actor.hasCamera()) {
			serializer.beginElement("Camera");

			Camera& camera = actor.getCameraCmp();
			serializer.writeAttribute("offset", camera.offset);

			serializer.endElement();
		}
		if (actor.hasLight()) {
			serializer.beginElement("Light");

			Light& light = actor.getLightCmp();
			serializer.writeAttribute("color", light.color);
			serializer.writeAttribute("radius", light.radius);
			serializer.writeAttribute("strength", light.strength);

			serializer.endElement();
		}
		if (actor.hasModel()) {
			serializer.beginElement("Model");

			Model& model = actor.getModelCmp();
			uint32_t i = 0;
			serializer.writeAttribute("meshCount", std::to_string(model.meshes.size()));
			for (auto mesh : model.meshes) {
				serializer.writeAttribute("mesh"+std::to_string(i), std::to_string(mesh.getHandle()));
				i++;
			}
			i = 0;
			serializer.writeAttribute("materialCount", std::to_string(model.materials.size()));
			for (auto material : model.materials) {
				serializer.writeAttribute("material"+std::to_string(i), std::to_string(material.getHandle()));
				i++;
			}

			serializer.endElement();
		}
		if (actor.hasRigidDynamic()) {
			serializer.beginElement("RigidDynamic");

			RigidDynamic& rigidDynamic = actor.getRigidDynamicCmp();

			serializer.endElement();
		}
		if (actor.hasRigidStatic()) {
			serializer.beginElement("RigidStatic");

			RigidStatic& rigidStatic = actor.getRigidStaticCmp();

			serializer.endElement();
		}
		if (actor.hasTransform()) {
			serializer.beginElement("Transform");

			Transform& transform = actor.getTransformCmp();
			serializer.writeAttribute("transform", transform.transform);

			serializer.endElement();
		}

		serializer.endElement();
		serializer.endSerialization();
	}
}
