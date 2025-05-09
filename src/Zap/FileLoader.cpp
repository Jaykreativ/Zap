#include "Zap/FileLoader.h"

#include "Zap/Scene/Model.h"
#include "Zap/Scene/Mesh.h"
#include "Zap/Physics/HitMesh.h"
#include "Zap/Scene/Material.h"
#include "Zap/Scene/Camera.h"
#include "Zap/Scene/Light.h"
#include "Zap/Physics/PhysicsComponent.h"
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

	Texture TextureLoader::load(std::filesystem::path filepath) {
		return load(filepath, UUID());
	}

	Texture TextureLoader::load(void* data, uint32_t width, uint32_t height, UUID handle) {
		auto& assetHandler = Base::getBase()->m_assetHandler;
		Texture texture = Texture(handle);
		texture.create();
		assetHandler.getTextureDataPtr(texture.getHandle())->image = ImageLoader::load(data, width, height);
		assetHandler.addLoadedTexture(texture);
		return texture;
	}

	Texture TextureLoader::load(std::filesystem::path filepath, UUID handle) {
		auto& assetHandler = Base::getBase()->m_assetHandler;
		int width, height, channels;
		stbi_set_flip_vertically_on_load(true);
		auto data = stbi_load(filepath.string().c_str(), &width, &height, &channels, 4);
		ZP_ASSERT(data, ("Image not loaded correctly: " + filepath.string()).c_str());
		auto texture = load(data, width, height, handle);
		assetHandler.registerTexture(texture, filepath);
		return texture;
	}

	Texture TextureLoader::load(const aiTexture* aiTexture, UUID handle) {
		ZP_ASSERT(!aiTexture->mHeight, "Raw texture loading not implemented Yet");// TODO implement raw texture loading
		auto& assetHandler = Base::getBase()->m_assetHandler;
		int width, height, channels;
		stbi_set_flip_vertically_on_load(true);
		auto data = stbi_load_from_memory((stbi_uc*)aiTexture->pcData, aiTexture->mWidth, &width, &height, &channels, 4);
		ZP_ASSERT(data, "Image not loaded correctly");
		return load(data, width, height, handle);
	}

	Texture TextureLoader::load(std::filesystem::path modelpath, std::filesystem::path textureID, UUID handle) {
		auto& assetHandler = Base::getBase()->m_assetHandler;
		Assimp::Importer importer;
		const aiScene* aScene = importer.ReadFile(modelpath.string().c_str(), 0);
		ZP_ASSERT(aScene, "Failed to load the modelfile for embedded texture");

		auto texture = load(aScene->GetEmbeddedTexture(textureID.string().c_str()), handle);
		assetHandler.registerTexture(texture, modelpath, textureID);
		return texture;
	}

	Material MaterialLoader::load(const aiScene* aScene, const aiMaterial* aMaterial, std::filesystem::path modelpath, UUID handle) {
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
				assetHandler.registerTexture(pMaterialData->albedoMap, modelpath, diffuseTexturePath.C_Str());
			}
			else {
				pMaterialData->albedoMap = TextureLoader::load(modelpath.remove_filename() / diffuseTexturePath.C_Str());
			}
			if (!ZP_IS_FLAG_ENABLED(flags, eTintTextures))
				pMaterialData->albedoColor = glm::vec4(1, 1, 1, 1);
		}
		if (aiGetMaterialTextureCount(aMaterial, aiTextureType_METALNESS) > 0) {
			aiString metallicTexturePath; aiGetMaterialTexture(aMaterial, aiTextureType_METALNESS, 0, &metallicTexturePath);
			auto embeddedTexture = aScene->GetEmbeddedTexture(metallicTexturePath.C_Str());
			if (embeddedTexture) {
				pMaterialData->metallicMap = TextureLoader::load(embeddedTexture);
				assetHandler.registerTexture(pMaterialData->metallicMap, modelpath, metallicTexturePath.C_Str());
			}
			else {
				pMaterialData->metallicMap = TextureLoader::load(modelpath.remove_filename() / metallicTexturePath.C_Str());
			}
			if (!ZP_IS_FLAG_ENABLED(flags, eTintTextures))
				pMaterialData->metallic = 1;
		}
		if (aiGetMaterialTextureCount(aMaterial, aiTextureType_DIFFUSE_ROUGHNESS) > 0) {
			aiString roughnessTexturePath; aiGetMaterialTexture(aMaterial, aiTextureType_DIFFUSE_ROUGHNESS, 0, &roughnessTexturePath);
			auto embeddedTexture = aScene->GetEmbeddedTexture(roughnessTexturePath.C_Str());
			if (embeddedTexture) {
				pMaterialData->roughnessMap = TextureLoader::load(embeddedTexture);
				assetHandler.registerTexture(pMaterialData->roughnessMap, modelpath, roughnessTexturePath.C_Str());
			}
			else {
				pMaterialData->roughnessMap = TextureLoader::load(modelpath.remove_filename() / roughnessTexturePath.C_Str());
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

	Mesh MeshLoader::loadFromFile(std::filesystem::path filepath, uint32_t index, glm::mat4& transform, UUID handle) {
		if (Base::getBase()->m_assetHandler.m_pathMeshMap.count({ filepath, index })) {
			return Base::getBase()->m_assetHandler.m_pathMeshMap.at({ filepath, index });
		}
		Assimp::Importer importer;
		const aiScene* aScene = importer.ReadFile(filepath.string().c_str(), aiProcess_Triangulate | aiProcess_GenUVCoords | aiProcess_GenBoundingBoxes);

		ZP_WARN(aScene, (std::string("Scene can't be loaded, check the filepath: ") + filepath.string()).c_str());
		if (!aScene)
			return Mesh(0);

		glm::vec3 boundMin, boundMax;
		auto mesh = load(aScene->mMeshes[index], transform, boundMin, boundMax, handle);
		Base::getBase()->m_assetHandler.m_loadedMeshes.push_back(mesh);
		Base::getBase()->m_assetHandler.registerMesh(mesh, filepath, index);
		return mesh;
	}

	HitMesh HitMeshLoader::load(std::filesystem::path filepath, uint32_t index) {
		return load(filepath, index, UUID());
	}
	HitMesh HitMeshLoader::load(std::filesystem::path filepath, uint32_t index, UUID handle) {
		auto* base = Base::getBase();
		if (base->m_assetHandler.m_pathHitMeshMap.count({filepath, index})) {
			return base->m_assetHandler.m_pathHitMeshMap.at({filepath, index});
		}
		Assimp::Importer importer;
		const aiScene* aScene = importer.ReadFile(filepath.string().c_str(), aiProcess_Triangulate);

		ZP_WARN(aScene, ("Scene can't be loaded, check the filepath: " + filepath.string()).c_str());
		if (!aScene)
			return HitMesh(0);

		auto hitMesh = load(aScene->mMeshes[index], handle);
		base->m_assetHandler.m_loadedHitMeshes.push_back(hitMesh);
		base->m_assetHandler.registerHitMesh(hitMesh, filepath, index);
		return hitMesh;
	}

	HitMesh HitMeshLoader::load(aiMesh* aMesh, UUID handle) {
		auto& assetHandler = Base::getBase()->m_assetHandler;

		HitMesh hitMesh = HitMesh(handle);
		assetHandler.m_hitMeshes[hitMesh.getHandle()] = HitMeshData{};
		auto* data = Base::getBase()->m_assetHandler.getHitMeshDataPtr(hitMesh.getHandle());

		data->m_vertexCount = aMesh->mNumVertices;
		data->m_vertices = new glm::vec3[data->m_vertexCount];
		for (size_t i = 0; i < aMesh->mNumVertices; i++) {
			data->m_vertices[i] = *reinterpret_cast<glm::vec3*>(&aMesh->mVertices[i]);
		}

		data->m_indexCount = aMesh->mNumFaces * 3;
		data->m_indices = new uint32_t[data->m_indexCount];
		for (size_t i = 0; i < aMesh->mNumFaces; i++) {
			data->m_indices[i*3+0] = aMesh->mFaces[i].mIndices[0];
			data->m_indices[i*3+1] = aMesh->mFaces[i].mIndices[1];
			data->m_indices[i*3+2] = aMesh->mFaces[i].mIndices[2];
		}

		return hitMesh;
	}

	Model ModelLoader::load(std::filesystem::path filepath) {
		Model model = {};
		model.filepath = filepath.string();

#ifdef _DEBUG
		auto timeStartLoad = std::chrono::high_resolution_clock::now();
#endif
		Assimp::Importer importer;
		const aiScene* aScene = importer.ReadFile(filepath.string().c_str(), aiProcess_Triangulate | aiProcess_GenUVCoords | aiProcess_GenBoundingBoxes);

		ZP_ASSERT(aScene, (std::string("Scene can't be loaded, check the filepath: ") + filepath.string()).c_str());

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

	void ModelLoader::processNode(const aiNode* node, const aiScene* aScene, std::filesystem::path path, glm::mat4& transform, Model& model) {
		auto& assetHandler = Base::getBase()->m_assetHandler;
		path = assetHandler.processPath(path);

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
				model.meshes.push_back(Base::getBase()->m_assetHandler.m_pathMeshMap.at({ path.string(), node->mMeshes[i]}));
			}
			else {
				// Load mesh
				model.meshes.push_back(MeshLoader::load(aScene->mMeshes[node->mMeshes[i]], newTransform, model.boundMin, model.boundMax));
				assetHandler.m_loadedMeshes.push_back(model.meshes.back());
				Base::getBase()->m_assetHandler.registerMesh(model.meshes.back(), path, node->mMeshes[i]);
			}

#ifdef _DEBUG
			auto timeMeshLoad = std::chrono::high_resolution_clock::now() - timeStartMeshLoad;
			auto timeStartMaterialLoad = std::chrono::high_resolution_clock::now();
#endif

			// Check if material already exists
			if (Base::getBase()->m_assetHandler.m_pathMaterialMap.count({ path.string(), aScene->mMeshes[node->mMeshes[i]]->mMaterialIndex})) {
				model.materials.push_back(Base::getBase()->m_assetHandler.m_pathMaterialMap.at({ path.string(), aScene->mMeshes[node->mMeshes[i]]->mMaterialIndex }));
			}
			else {
				// Load material
				aiMaterial* aMaterial = aScene->mMaterials[aScene->mMeshes[node->mMeshes[i]]->mMaterialIndex];
				model.materials.push_back(MaterialLoader::load(aScene, aMaterial, path));
				assetHandler.m_loadedMaterials.push_back(model.materials.back());
				Base::getBase()->m_assetHandler.registerMaterial(model.materials.back(), path, aScene->mMeshes[node->mMeshes[i]]->mMaterialIndex);
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

	void loadCamera(Serializer& serializer, Actor actor) {
		glm::mat4 offset = serializer.readAttributeMat4("offset");
		actor.addCamera(offset);
	}

	void loadLight(Serializer& serializer, Actor actor) {
		glm::vec3 color = serializer.readAttributeVec3("color");;
		float strength = serializer.readAttributef("strength");
		float radius = serializer.readAttributef("radius");

		actor.addLight(color, strength, radius);
	}

	void loadModel(Serializer& serializer, Actor actor) {
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
	}

	/* Geometries */

	void loadSphereGeometry(Serializer& serializer, std::unique_ptr<PhysicsGeometry>& upGeometry) {
		float radius = serializer.readAttributef("radius");
		upGeometry = std::make_unique<SphereGeometry>(radius);
	}

	void loadCapsuleGeometry(Serializer& serializer, std::unique_ptr<PhysicsGeometry>& upGeometry) {
		float radius = serializer.readAttributef("radius");
		float halfHeight = serializer.readAttributef("halfHeight");
		upGeometry = std::make_unique<CapsuleGeometry>(radius, halfHeight);
	}

	void loadBoxGeometry(Serializer& serializer, std::unique_ptr<PhysicsGeometry>& upGeometry) {
		glm::vec3 halfExtents = serializer.readAttributeVec3("halfExtents");
		upGeometry = std::make_unique<BoxGeometry>(halfExtents);
	}

	void loadPlaneGeometry(Serializer& serializer, std::unique_ptr<PhysicsGeometry>& upGeometry) {
		upGeometry = std::make_unique<PlaneGeometry>();
	}

	void loadConvexMeshGeometry(Serializer& serializer, std::unique_ptr<PhysicsGeometry>& upGeometry) {
		HitMesh hitMesh = serializer.readAttributeUUID("hitMesh");
		ConvexMesh convexMesh(hitMesh);
		upGeometry = std::make_unique<ConvexMeshGeometry>(convexMesh);
		//convexMesh.release();
	}

	void loadShape(Serializer& serializer, std::vector<Shape>& shapes, size_t index) {
		bool success = true;
		glm::mat4 localPose = serializer.readAttributeMat4("localPose", &success);
		if (!success)
			localPose = glm::mat4(1);
		serializer.beginElement("Material");
		float dynamicFriction = serializer.readAttributef("dynamicFriction");
		float staticFriction = serializer.readAttributef("staticFriction");
		float restitution = serializer.readAttributef("restitution");
		serializer.endElement();
		PhysicsMaterial material(staticFriction, dynamicFriction, restitution);
		int type = serializer.readAttributei("type");
		std::unique_ptr<PhysicsGeometry> upGeometry;
		switch (type) {
		case eGEOMETRY_TYPE_SPHERE:
			loadSphereGeometry(serializer, upGeometry);
			break;
		case eGEOMETRY_TYPE_CAPSULE:
			loadCapsuleGeometry(serializer, upGeometry);
			break;
		case eGEOMETRY_TYPE_BOX:
			loadBoxGeometry(serializer, upGeometry);
			break;
		case eGEOMETRY_TYPE_PLANE:
			loadPlaneGeometry(serializer, upGeometry);
			break;
		case eGEOMETRY_TYPE_CONVEX_MESH:
			loadConvexMeshGeometry(serializer, upGeometry);
			break;
		default:
			ZP_WARN(false, "Failed to load shape, invalid geometry type");
			return;
		}
		shapes[index] = Shape(*upGeometry.get(), material, true, localPose);
	}

	void loadRigidDynamic(Serializer& serializer, Actor actor) {
		size_t shapeCount = serializer.readAttributeull("shapeCount");
		std::vector<Shape> shapes(shapeCount);
		for (size_t i = 0; i < shapeCount; i++) {
			serializer.beginElement("Shape" + std::to_string(i));
			loadShape(serializer, shapes, i);
			serializer.endElement();
		}
		actor.addRigidDynamic(shapes);
	}

	void loadRigidStatic(Serializer& serializer, Actor actor) {
		size_t shapeCount = serializer.readAttributeull("shapeCount");
		std::vector<Shape> shapes(shapeCount);
		for (size_t i = 0; i < shapeCount; i++) {
			serializer.beginElement("Shape" + std::to_string(i));
			loadShape(serializer, shapes, i);
			serializer.endElement();
		}
		actor.addRigidStatic(shapes);
	}

	void loadTransform(Serializer& serializer, Actor actor) {
		glm::mat4 transform = serializer.readAttributeMat4("transform");
		actor.addTransform(transform);
	}

	Actor ActorLoader::load(std::filesystem::path filepath, Scene* pScene) {
		Serializer serializer;
		Actor actor = Actor((UUID)0, pScene);
		if (serializer.beginDeserialization(filepath.c_str()) && serializer.beginElement("Actor")) {
			UUID handle;
			if (ZP_IS_FLAG_ENABLED(flags, eReuseActor))
				handle = UUID();
			else
				handle = std::stoull(serializer.readAttribute("handle"));
			actor = Actor(handle, pScene);
			pScene->attachActor(actor);

			if (serializer.beginElement("Transform")) {
				loadTransform(serializer, actor);
				serializer.endElement();
			}
			if (serializer.beginElement("Camera")) {
				loadCamera(serializer, actor);
				serializer.endElement();
			}
			if (serializer.beginElement("Light")) {
				loadLight(serializer, actor);
				serializer.endElement();
			}
			if (serializer.beginElement("Model")) {
				loadModel(serializer, actor);
				serializer.endElement();
			}
			if (serializer.beginElement("RigidDynamic")) {
				loadRigidDynamic(serializer, actor);
				serializer.endElement();
			}
			if (serializer.beginElement("RigidStatic")) {
				loadRigidStatic(serializer, actor);
				serializer.endElement();
			}

			serializer.endElement();
			serializer.endDeserialization();
		}
		else
			ZP_WARN(false, ("Failed loading actor, filepath: " + filepath.string()).c_str());
		return actor;
	}

	void writeCamera(Serializer& serializer, Camera& camera) {
		serializer.writeAttribute("offset", camera.offset);
	}

	void writeLight(Serializer& serializer, Light& light) {
		serializer.writeAttribute("color", light.color);
		serializer.writeAttribute("radius", light.radius);
		serializer.writeAttribute("strength", light.strength);
	}

	void writeModel(Serializer& serializer, Model& model) {
		uint32_t i = 0;
		serializer.writeAttribute("meshCount", std::to_string(model.meshes.size()));
		for (auto mesh : model.meshes) {
			serializer.writeAttribute("mesh" + std::to_string(i), std::to_string(mesh.getHandle()));
			i++;
		}
		i = 0;
		serializer.writeAttribute("materialCount", std::to_string(model.materials.size()));
		for (auto material : model.materials) {
			serializer.writeAttribute("material" + std::to_string(i), std::to_string(material.getHandle()));
			i++;
		}
	}

	/* Geometries */

	void writeSphereGeometry(Serializer& serializer, SphereGeometry& geometry) {
		serializer.writeAttribute("radius", geometry.getRadius());
	}

	void writeCapsuleGeometry(Serializer& serializer, CapsuleGeometry& geometry) {
		serializer.writeAttribute("radius", geometry.getRadius());
		serializer.writeAttribute("halfHeight", geometry.getHalfHeight());
	}

	void writeBoxGeometry(Serializer& serializer, BoxGeometry& geometry) {
		serializer.writeAttribute("halfExtents", geometry.getHalfExtents());
	}

	void writePlaneGeometry(Serializer& serializer, PlaneGeometry& geometry) {}

	void writeConvexMeshGeometry(Serializer& serializer, ConvexMeshGeometry& geometry) {
		serializer.writeAttribute("hitMesh", geometry.getHitMesh().getHandle());
		ZP_WARN(false, "HitMesh cannot be retrieved from the shape alone, TODO store hit mesh identifier");
	}

	void writeShape(Serializer& serializer, Shape shape) {
		serializer.writeAttribute("localPose", shape.getLocalPose());
		auto material = shape.getMaterial();
		serializer.beginElement("Material");
		serializer.writeAttribute("dynamicFriction", material.getDynamicFriction());
		serializer.writeAttribute("staticFriction", material.getStaticFriction());
		serializer.writeAttribute("restitution", material.getRestitution());
		serializer.endElement();
		auto upGeometry = shape.getGeometry();
		int type = upGeometry->getType();
		serializer.writeAttribute("type", type);
		switch (type) {
		case eGEOMETRY_TYPE_SPHERE:
			writeSphereGeometry(serializer, *dynamic_cast<SphereGeometry*>(upGeometry.get()));
			break;
		case eGEOMETRY_TYPE_CAPSULE:
			writeCapsuleGeometry(serializer, *dynamic_cast<CapsuleGeometry*>(upGeometry.get()));
			break;
		case eGEOMETRY_TYPE_BOX:
			writeBoxGeometry(serializer, *dynamic_cast<BoxGeometry*>(upGeometry.get()));
			break;
		case eGEOMETRY_TYPE_PLANE:
			writePlaneGeometry(serializer, *dynamic_cast<PlaneGeometry*>(upGeometry.get()));
			break;
		case eGEOMETRY_TYPE_CONVEX_MESH:
			writeConvexMeshGeometry(serializer, *dynamic_cast<ConvexMeshGeometry*>(upGeometry.get()));
			break;
		default:
			break;
		}
	}

	void writeRigidDynamic(Serializer& serializer, Actor actor, RigidDynamic& rigidDynamic) {
		auto shapes = actor.cmpRigidDynamic_getShapes();
		serializer.writeAttribute("shapeCount", shapes.size());
		size_t i = 0;
		for (auto shape : shapes) {
			serializer.beginElement("Shape" + std::to_string(i));
			writeShape(serializer, shape);
			serializer.endElement();
			i++;
		}
	}
	
	void writeRigidStatic(Serializer& serializer, Actor actor, RigidStatic& rigidStatic) {
		auto shapes = actor.cmpRigidStatic_getShapes();
		serializer.writeAttribute("shapeCount", shapes.size());
		size_t i = 0;
		for (auto shape : shapes) {
			serializer.beginElement("Shape" + std::to_string(i));
			writeShape(serializer, shape);
			serializer.endElement();
			i++;
		}
	}

	void writeTransform(Serializer& serializer, Transform& transform) {
		serializer.writeAttribute("transform", transform.transform);
	}

	void ActorLoader::store(std::filesystem::path filepath, Actor actor) {
		Serializer serializer;
		serializer.beginSerialization(filepath.c_str());
		serializer.beginElement("Actor");

		serializer.writeAttribute("handle", std::to_string(actor.getHandle()));

		if (actor.hasCamera()) {
			serializer.beginElement("Camera");

			Camera& camera = actor.getCameraCmp();
			writeCamera(serializer, camera);

			serializer.endElement();
		}
		if (actor.hasLight()) {
			serializer.beginElement("Light");

			Light& light = actor.getLightCmp();
			writeLight(serializer, light);

			serializer.endElement();
		}
		if (actor.hasModel()) {
			serializer.beginElement("Model");

			Model& model = actor.getModelCmp();
			writeModel(serializer, model);

			serializer.endElement();
		}
		if (actor.hasRigidDynamic()) {
			serializer.beginElement("RigidDynamic");

			RigidDynamic& rigidDynamic = actor.getRigidDynamicCmp();
			writeRigidDynamic(serializer, actor, rigidDynamic);

			serializer.endElement();
		}
		if (actor.hasRigidStatic()) {
			serializer.beginElement("RigidStatic");

			RigidStatic& rigidStatic = actor.getRigidStaticCmp();
			writeRigidStatic(serializer, actor, rigidStatic);

			serializer.endElement();
		}
		if (actor.hasTransform()) {
			serializer.beginElement("Transform");

			Transform& transform = actor.getTransformCmp();
			writeTransform(serializer, transform);

			serializer.endElement();
		}

		serializer.endElement();
		serializer.endSerialization();
	}
}
