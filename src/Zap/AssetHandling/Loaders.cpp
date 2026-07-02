#include "Zap/AssetHandling/Loaders.h"

#include "Zap/AssetHandling/FileLinker.h"
#include "Zap/AssetHandling/AssetTypes/Mesh.h"
#include "Zap/AssetHandling/AssetTypes/Material.h"
#include "Zap/AssetHandling/AssetTypes/Texture.h"
#include "Zap/AssetHandling/AssetTypes/HitMesh.h"
#include "Zap/Rendering/stb_image.h"

#include "assimp/Importer.hpp"
#include "assimp/scene.h"
#include "assimp/postprocess.h"

#include <sstream>
#include <fstream>

namespace Zap {
	// filetype support
	std::vector<std::string> Loader::supportedFileExtensions() {
		return {
			// means not tested TODO test formats
			".obj",
			".glb",
			//".gltf",
			".png",
			".jpeg",
			".jpg"
			//".tga"
			//".bmp"
			//".psd"
			//".gif"
			//".hdr"
			//".pic"
			//".pnm"
		};
	}
	std::vector<std::string> ModelLoader::supportedFileExtensions() {
		return {
			".obj",
			".glb",
		};
	}
	std::vector<std::string> TextureLoader::supportedFileExtensions() {
		return {
			".png",
			".jpeg",
			".jpg"
		};
	}

	Loader::Loader() 
		: m_assetHandler(*Base::getBase()->getAssetHandler())
	{}
	
	Loader::~Loader(){}

	void Loader::load(std::filesystem::path path) {
		std::string extension = path.extension().string();
		ZP_WARN(isFileSupported(path), "Filetype not supported | " << extension << " | Loader::load");
		// choose subload
		if (
			extension == ".obj" ||
			extension == ".glb")
			assimpLoad(path);
		if (
			extension == ".png" ||
			extension == ".jpeg" ||
			extension == ".jpg")
			stbImageLoad(path);
	}

	bool Loader::isFileSupported(std::filesystem::path path) {
		std::string extension = path.extension().string();
		auto list = supportedFileExtensions();
		for (std::string str : list)
			if (str == extension)
				return true;
		return false;
	}

	AssetHandle<Mesh> Loader::extractMesh(const aiMesh* aMesh, glm::mat4 transform, Model& model) {
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
				if (aMesh->mTextureCoords[0])
					data[i].texCoords = *((glm::vec2*)&aMesh->mTextureCoords[0][i]);
				else
					data[i].texCoords = { 0, 0 };
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


		auto vertexBuffer = vk::Buffer(vertexStgBuffer.getSize(), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT
			| VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR
		);
		vertexBuffer.init(); vertexBuffer.allocate(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
		vertexBuffer.uploadData(&vertexStgBuffer);

		auto indexBuffer = vk::Buffer(indexStgBuffer.getSize(), VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT
			| VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR
		);
		indexBuffer.init(); indexBuffer.allocate(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
		indexBuffer.uploadData(&indexStgBuffer);


		vertexStgBuffer.destroy();
		indexStgBuffer.destroy();

		// Bounding box
		glm::vec3 boundMin = transform * glm::vec4(*((glm::vec3*)&aMesh->mAABB.mMin), 1);
		glm::vec3 boundMax = transform * glm::vec4(*((glm::vec3*)&aMesh->mAABB.mMax), 1);

		model.boundMin = glm::min(model.boundMin, boundMin);
		model.boundMax = glm::max(model.boundMax, boundMax);

		auto mesh = m_assetHandler.generateAsset<Mesh>(transform, vertexBuffer, indexBuffer, boundMax, boundMin);
	}

	AssetHandle<Material> Loader::extractMaterial(const aiMaterial* aMaterial, const aiScene* aScene, std::filesystem::path path) {
		// base material
		aiColor4D aDiffuse; aiGetMaterialColor(aMaterial, AI_MATKEY_COLOR_DIFFUSE, &aDiffuse);
		auto albedoColor = glm::vec4(aDiffuse.r, aDiffuse.g, aDiffuse.b, 1);
		float metallic; aiGetMaterialFloat(aMaterial, AI_MATKEY_METALLIC_FACTOR, &metallic);
		float roughness; aiGetMaterialFloat(aMaterial, AI_MATKEY_ROUGHNESS_FACTOR, &roughness);
		aiColor4D aEmissive; aiGetMaterialColor(aMaterial, AI_MATKEY_COLOR_EMISSIVE, &aEmissive);
		auto emissive = glm::vec4(aEmissive.r, aEmissive.g, aEmissive.b, 0);
		aiGetMaterialFloat(aMaterial, AI_MATKEY_EMISSIVE_INTENSITY, &emissive.w);

		// material maps / textures
		AssetHandle<Texture> albedoMap;
		AssetHandle<Texture> metallicMap;
		AssetHandle<Texture> roughnessMap;
		if (aiGetMaterialTextureCount(aMaterial, aiTextureType_DIFFUSE) > 0) {
			aiString diffuseTexturePath; aiGetMaterialTexture(aMaterial, aiTextureType_DIFFUSE, 0, &diffuseTexturePath);
			auto embeddedTexture = aScene->GetEmbeddedTexture(diffuseTexturePath.C_Str());
			if (embeddedTexture) {
				ZP_WARN(false, "embedded textures are WIP");
			}
			else {
				TextureLoader loader;
				loader.load(path.remove_filename() / diffuseTexturePath.C_Str());
				albedoMap = loader.result();
			}
			albedoColor = glm::vec4(1, 1, 1, 1);
		}
		if (aiGetMaterialTextureCount(aMaterial, aiTextureType_METALNESS) > 0) {
			aiString metallicTexturePath; aiGetMaterialTexture(aMaterial, aiTextureType_METALNESS, 0, &metallicTexturePath);
			auto embeddedTexture = aScene->GetEmbeddedTexture(metallicTexturePath.C_Str());
			if (embeddedTexture) {
				ZP_WARN(false, "embedded textures are WIP");
			}
			else {
				TextureLoader loader;
				loader.load(path.remove_filename() / metallicTexturePath.C_Str());
				metallicMap = loader.result();
			}
			metallic = 1;
		}
		if (aiGetMaterialTextureCount(aMaterial, aiTextureType_DIFFUSE_ROUGHNESS) > 0) {
			aiString roughnessTexturePath; aiGetMaterialTexture(aMaterial, aiTextureType_DIFFUSE_ROUGHNESS, 0, &roughnessTexturePath);
			auto embeddedTexture = aScene->GetEmbeddedTexture(roughnessTexturePath.C_Str());
			if (embeddedTexture) {
				ZP_WARN(false, "embedded textures are WIP");
			}
			else {
				TextureLoader loader;
				loader.load(path.remove_filename() / roughnessTexturePath.C_Str());
				roughnessMap = loader.result();
			}
			roughness = 1;
		}

		auto material = m_assetHandler.generateAsset<Material>(albedoColor, metallic, roughness, emissive, albedoMap, metallicMap, roughnessMap);
		return material;
	}

	void Loader::processNode(FileLinker::FileLink<AssimpReconstructionData>& fileLink, const aiNode* node, const aiScene* aScene, std::filesystem::path path, glm::mat4& transform, Model& model) {
		glm::mat4 newTransform = transform * AssimpUtils::mat4ToGlmMat4(node->mTransformation);
		for (uint32_t i = 0; i < node->mNumMeshes; i++) {
			// Mesh
			auto mesh = extractMesh(aScene->mMeshes[node->mMeshes[i]], transform, model);
			fileLink.registerAsset(mesh);
			model.meshes.push_back(mesh);

			// Material
			auto material = extractMaterial(aScene->mMaterials[aScene->mMeshes[node->mMeshes[i]]->mMaterialIndex], aScene, path);
			fileLink.registerAsset(material);
			model.materials.push_back(material);
		}

		for (uint32_t i = 0; i < node->mNumChildren; i++) {
			processNode(fileLink, node->mChildren[i], aScene, path, newTransform, model);
		}
	}


	void Loader::assimpLoad(std::filesystem::path path) {
		auto fileLinker = m_assetHandler.getFileLinker();
		if (fileLinker.isRegistered(path)) {
			auto rec = fileLinker.getReconstructionData<AssimpReconstructionData>(path);
			submitModel(rec->model);
		}
		else {
			auto fileLink = fileLinker.beginFileLinking<AssimpReconstructionData>(path);

			Assimp::Importer importer;
			const aiScene* aScene = importer.ReadFile(path.string().c_str(), aiProcess_Triangulate | aiProcess_GenUVCoords | aiProcess_GenBoundingBoxes);

			ZP_ASSERT(aScene, (std::string("Scene can't be loaded, check the filepath: ") + path.string()).c_str());

			Model model;
			processNode(fileLink, aScene->mRootNode, aScene, path, glm::mat4(1), model);
			submitModel(model);
			fileLink->model = model;

			fileLinker.endFileLinking(fileLink);
		}
	}

	void Loader::stbImageLoad(std::filesystem::path path) {
		auto fileLinker = m_assetHandler.getFileLinker();

		// check if file has already been loaded
		if (fileLinker.isRegistered(path)) {
			auto rec = fileLinker.getReconstructionData<StbImageReconstructionData>(path);
			submitTexture(rec->texture);
		}
		else {
			auto fileLink = fileLinker.beginFileLinking<StbImageReconstructionData>(path);

			// fileload
			int width, height, channels;
			stbi_set_flip_vertically_on_load(true);
			auto data = stbi_load(path.string().c_str(), &width, &height, &channels, 4);
			ZP_ASSERT(data, ("Image not loaded correctly: " + path.string()).c_str());

			// upload
			auto base = Base::getBase();
			Image2D image(
				VK_FORMAT_R8G8B8A8_UNORM,
				VkExtent2D{ (unsigned int)width, (unsigned int)height },
				VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
				VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
			);
			image.uploadData(width * height * 4, data);

			auto texture = m_assetHandler.generateAsset<Texture>(std::move(image));
			fileLink.registerAsset(texture);
			fileLink->texture = texture; // save texture handle in case this file is being loaded again
			submitTexture(texture);

			fileLinker.endFileLinking(fileLink);
		}
	}

	// TODO load hitmeshes
	//AssetHandle<HitMesh> HitMeshLoader::load(std::filesystem::path filepath, uint32_t index) {
	//	return load(filepath, index, UUID());
	//}
	//AssetHandle<HitMesh> HitMeshLoader::load(std::filesystem::path filepath, uint32_t index, UUID handle) {
	//	auto* base = Base::getBase();
	//	Assimp::Importer importer;
	//	const aiScene* aScene = importer.ReadFile(filepath.string().c_str(), aiProcess_Triangulate);
	//
	//	ZP_WARN(aScene, ("Scene can't be loaded, check the filepath: " + filepath.string()).c_str());
	//	if (!aScene)
	//		return AssetHandle<HitMesh>(); // return null handle
	//
	//	auto hitMesh = load(aScene->mMeshes[index], handle);
	//	return hitMesh;
	//}
	//
	//AssetHandle<HitMesh> HitMeshLoader::load(aiMesh* aMesh, UUID handle) {
	//	auto& assetHandler = Base::getBase()->m_assetHandler;
	//
	//	auto hitMesh = assetHandler->createAsset<HitMesh>(handle);
	//
	//	hitMesh->m_vertexCount = aMesh->mNumVertices;
	//	hitMesh->m_vertices = new glm::vec3[hitMesh->m_vertexCount];
	//	for (size_t i = 0; i < aMesh->mNumVertices; i++) {
	//		hitMesh->m_vertices[i] = *reinterpret_cast<glm::vec3*>(&aMesh->mVertices[i]);
	//	}
	//
	//	hitMesh->m_indexCount = aMesh->mNumFaces * 3;
	//	hitMesh->m_indices = new uint32_t[hitMesh->m_indexCount];
	//	for (size_t i = 0; i < aMesh->mNumFaces; i++) {
	//		hitMesh->m_indices[i*3+0] = aMesh->mFaces[i].mIndices[0];
	//		hitMesh->m_indices[i*3+1] = aMesh->mFaces[i].mIndices[1];
	//		hitMesh->m_indices[i*3+2] = aMesh->mFaces[i].mIndices[2];
	//	}
	//
	//	return hitMesh;
	//}

//	void loadCamera(Serializer& serializer, Actor actor) {
//		glm::mat4 offset = serializer.readAttributeMat4("offset");
//		actor.addCamera(offset);
//	}
//
//	void loadLight(Serializer& serializer, Actor actor) {
//		glm::vec3 color = serializer.readAttributeVec3("color");;
//		float strength = serializer.readAttributef("strength");
//		float radius = serializer.readAttributef("radius");
//
//		actor.addLight(color, strength, radius);
//	}
//
//	void loadModel(Serializer& serializer, Actor actor) {
//		Model model;
//		int meshCount = serializer.readAttributei("meshCount");
//		for (size_t i = 0; i < meshCount; i++) {
//			UUID handle = std::stoull(serializer.readAttribute("mesh" + std::to_string(i)));
//			model.meshes.push_back(AssetHandle<Mesh>());
//		}
//
//		int materialCount = serializer.readAttributei("materialCount");
//		for (size_t i = 0; i < materialCount; i++) {
//			UUID handle = std::stoull(serializer.readAttribute("material" + std::to_string(i)));
//			model.materials.push_back(Material(handle));
//		}
//
//		actor.addModel(model);
//	}
//
//	/* Geometries */
//
//	void loadSphereGeometry(Serializer& serializer, std::unique_ptr<PhysicsGeometry>& upGeometry) {
//		float radius = serializer.readAttributef("radius");
//		upGeometry = std::make_unique<SphereGeometry>(radius);
//	}
//
//	void loadCapsuleGeometry(Serializer& serializer, std::unique_ptr<PhysicsGeometry>& upGeometry) {
//		float radius = serializer.readAttributef("radius");
//		float halfHeight = serializer.readAttributef("halfHeight");
//		upGeometry = std::make_unique<CapsuleGeometry>(radius, halfHeight);
//	}
//
//	void loadBoxGeometry(Serializer& serializer, std::unique_ptr<PhysicsGeometry>& upGeometry) {
//		glm::vec3 halfExtents = serializer.readAttributeVec3("halfExtents");
//		upGeometry = std::make_unique<BoxGeometry>(halfExtents);
//	}
//
//	void loadPlaneGeometry(Serializer& serializer, std::unique_ptr<PhysicsGeometry>& upGeometry) {
//		upGeometry = std::make_unique<PlaneGeometry>();
//	}
//
//	void loadConvexMeshGeometry(Serializer& serializer, std::unique_ptr<PhysicsGeometry>& upGeometry) {
//		HitMesh hitMesh = serializer.readAttributeUUID("hitMesh");
//		ConvexMesh convexMesh(hitMesh);
//		upGeometry = std::make_unique<ConvexMeshGeometry>(convexMesh);
//		//convexMesh.release();
//	}
//
//	void loadShape(Serializer& serializer, std::vector<Shape>& shapes, size_t index) {
//		bool success = true;
//		glm::mat4 localPose = serializer.readAttributeMat4("localPose", &success);
//		if (!success)
//			localPose = glm::mat4(1);
//		serializer.beginElement("Material");
//		float dynamicFriction = serializer.readAttributef("dynamicFriction");
//		float staticFriction = serializer.readAttributef("staticFriction");
//		float restitution = serializer.readAttributef("restitution");
//		serializer.endElement();
//		PhysicsMaterial material(staticFriction, dynamicFriction, restitution);
//		int type = serializer.readAttributei("type");
//		std::unique_ptr<PhysicsGeometry> upGeometry;
//		switch (type) {
//		case eGEOMETRY_TYPE_SPHERE:
//			loadSphereGeometry(serializer, upGeometry);
//			break;
//		case eGEOMETRY_TYPE_CAPSULE:
//			loadCapsuleGeometry(serializer, upGeometry);
//			break;
//		case eGEOMETRY_TYPE_BOX:
//			loadBoxGeometry(serializer, upGeometry);
//			break;
//		case eGEOMETRY_TYPE_PLANE:
//			loadPlaneGeometry(serializer, upGeometry);
//			break;
//		case eGEOMETRY_TYPE_CONVEX_MESH:
//			loadConvexMeshGeometry(serializer, upGeometry);
//			break;
//		default:
//			ZP_WARN(false, "Failed to load shape, invalid geometry type");
//			return;
//		}
//		shapes[index] = Shape(*upGeometry.get(), material, true, localPose);
//	}
//
//	void loadRigidDynamic(Serializer& serializer, Actor actor) {
//		size_t shapeCount = serializer.readAttributeull("shapeCount");
//		std::vector<Shape> shapes(shapeCount);
//		for (size_t i = 0; i < shapeCount; i++) {
//			serializer.beginElement("Shape" + std::to_string(i));
//			loadShape(serializer, shapes, i);
//			serializer.endElement();
//		}
//		actor.addRigidDynamic(shapes);
//	}
//
//	void loadRigidStatic(Serializer& serializer, Actor actor) {
//		size_t shapeCount = serializer.readAttributeull("shapeCount");
//		std::vector<Shape> shapes(shapeCount);
//		for (size_t i = 0; i < shapeCount; i++) {
//			serializer.beginElement("Shape" + std::to_string(i));
//			loadShape(serializer, shapes, i);
//			serializer.endElement();
//		}
//		actor.addRigidStatic(shapes);
//	}
//
//	void loadTransform(Serializer& serializer, Actor actor) {
//		glm::mat4 transform = serializer.readAttributeMat4("transform");
//		actor.addTransform(transform);
//	}
//
//	Actor ActorLoader::load(std::filesystem::path filepath, Scene* pScene) {
//		Serializer serializer;
//		Actor actor = Actor((UUID)0, pScene);
//		if (serializer.beginDeserialization(filepath.c_str()) && serializer.beginElement("Actor")) {
//			UUID handle;
//			if (ZP_IS_FLAG_ENABLED(flags, eReuseActor))
//				handle = UUID();
//			else
//				handle = std::stoull(serializer.readAttribute("handle"));
//			actor = Actor(handle, pScene);
//			pScene->attachActor(actor);
//
//			if (serializer.beginElement("Transform")) {
//				loadTransform(serializer, actor);
//				serializer.endElement();
//			}
//			if (serializer.beginElement("Camera")) {
//				loadCamera(serializer, actor);
//				serializer.endElement();
//			}
//			if (serializer.beginElement("Light")) {
//				loadLight(serializer, actor);
//				serializer.endElement();
//			}
//			if (serializer.beginElement("Model")) {
//				loadModel(serializer, actor);
//				serializer.endElement();
//			}
//			if (serializer.beginElement("RigidDynamic")) {
//				loadRigidDynamic(serializer, actor);
//				serializer.endElement();
//			}
//			if (serializer.beginElement("RigidStatic")) {
//				loadRigidStatic(serializer, actor);
//				serializer.endElement();
//			}
//
//			serializer.endElement();
//			serializer.endDeserialization();
//		}
//		else
//			ZP_WARN(false, ("Failed loading actor, filepath: " + filepath.string()).c_str());
//		return actor;
//	}
//
//	void writeCamera(Serializer& serializer, Camera& camera) {
//		serializer.writeAttribute("offset", camera.offset);
//	}
//
//	void writeLight(Serializer& serializer, Light& light) {
//		serializer.writeAttribute("color", light.color);
//		serializer.writeAttribute("radius", light.radius);
//		serializer.writeAttribute("strength", light.strength);
//	}
//
//	void writeModel(Serializer& serializer, Model& model) {
//		uint32_t i = 0;
//		serializer.writeAttribute("meshCount", std::to_string(model.meshes.size()));
//		for (auto mesh : model.meshes) {
//			serializer.writeAttribute("mesh" + std::to_string(i), std::to_string(mesh.getHandle()));
//			i++;
//		}
//		i = 0;
//		serializer.writeAttribute("materialCount", std::to_string(model.materials.size()));
//		for (auto material : model.materials) {
//			serializer.writeAttribute("material" + std::to_string(i), std::to_string(material.getHandle()));
//			i++;
//		}
//	}
//
//	/* Geometries */
//
//	void writeSphereGeometry(Serializer& serializer, SphereGeometry& geometry) {
//		serializer.writeAttribute("radius", geometry.getRadius());
//	}
//
//	void writeCapsuleGeometry(Serializer& serializer, CapsuleGeometry& geometry) {
//		serializer.writeAttribute("radius", geometry.getRadius());
//		serializer.writeAttribute("halfHeight", geometry.getHalfHeight());
//	}
//
//	void writeBoxGeometry(Serializer& serializer, BoxGeometry& geometry) {
//		serializer.writeAttribute("halfExtents", geometry.getHalfExtents());
//	}
//
//	void writePlaneGeometry(Serializer& serializer, PlaneGeometry& geometry) {}
//
//	void writeConvexMeshGeometry(Serializer& serializer, ConvexMeshGeometry& geometry) {
//		serializer.writeAttribute("hitMesh", geometry.getHitMesh().getHandle());
//		ZP_WARN(false, "HitMesh cannot be retrieved from the shape alone, TODO store hit mesh identifier");
//	}
//
//	void writeShape(Serializer& serializer, Shape shape) {
//		serializer.writeAttribute("localPose", shape.getLocalPose());
//		auto material = shape.getMaterial();
//		serializer.beginElement("Material");
//		serializer.writeAttribute("dynamicFriction", material.getDynamicFriction());
//		serializer.writeAttribute("staticFriction", material.getStaticFriction());
//		serializer.writeAttribute("restitution", material.getRestitution());
//		serializer.endElement();
//		auto upGeometry = shape.getGeometry();
//		int type = upGeometry->getType();
//		serializer.writeAttribute("type", type);
//		switch (type) {
//		case eGEOMETRY_TYPE_SPHERE:
//			writeSphereGeometry(serializer, *dynamic_cast<SphereGeometry*>(upGeometry.get()));
//			break;
//		case eGEOMETRY_TYPE_CAPSULE:
//			writeCapsuleGeometry(serializer, *dynamic_cast<CapsuleGeometry*>(upGeometry.get()));
//			break;
//		case eGEOMETRY_TYPE_BOX:
//			writeBoxGeometry(serializer, *dynamic_cast<BoxGeometry*>(upGeometry.get()));
//			break;
//		case eGEOMETRY_TYPE_PLANE:
//			writePlaneGeometry(serializer, *dynamic_cast<PlaneGeometry*>(upGeometry.get()));
//			break;
//		case eGEOMETRY_TYPE_CONVEX_MESH:
//			writeConvexMeshGeometry(serializer, *dynamic_cast<ConvexMeshGeometry*>(upGeometry.get()));
//			break;
//		default:
//			break;
//		}
//	}
//
//	void writeRigidDynamic(Serializer& serializer, Actor actor, RigidDynamic& rigidDynamic) {
//		auto shapes = actor.cmpRigidDynamic_getShapes();
//		serializer.writeAttribute("shapeCount", shapes.size());
//		size_t i = 0;
//		for (auto shape : shapes) {
//			serializer.beginElement("Shape" + std::to_string(i));
//			writeShape(serializer, shape);
//			serializer.endElement();
//			i++;
//		}
//	}
//	
//	void writeRigidStatic(Serializer& serializer, Actor actor, RigidStatic& rigidStatic) {
//		auto shapes = actor.cmpRigidStatic_getShapes();
//		serializer.writeAttribute("shapeCount", shapes.size());
//		size_t i = 0;
//		for (auto shape : shapes) {
//			serializer.beginElement("Shape" + std::to_string(i));
//			writeShape(serializer, shape);
//			serializer.endElement();
//			i++;
//		}
//	}
//
//	void writeTransform(Serializer& serializer, Transform& transform) {
//		serializer.writeAttribute("transform", transform.transform);
//	}
//
//	void ActorLoader::store(std::filesystem::path filepath, Actor actor) {
//		Serializer serializer;
//		serializer.beginSerialization(filepath.c_str());
//		serializer.beginElement("Actor");
//
//		serializer.writeAttribute("handle", std::to_string(actor.getHandle()));
//
//		if (actor.hasCamera()) {
//			serializer.beginElement("Camera");
//
//			Camera& camera = actor.getCameraCmp();
//			writeCamera(serializer, camera);
//
//			serializer.endElement();
//		}
//		if (actor.hasLight()) {
//			serializer.beginElement("Light");
//
//			Light& light = actor.getLightCmp();
//			writeLight(serializer, light);
//
//			serializer.endElement();
//		}
//		if (actor.hasModel()) {
//			serializer.beginElement("Model");
//
//			Model& model = actor.getModelCmp();
//			writeModel(serializer, model);
//
//			serializer.endElement();
//		}
//		if (actor.hasRigidDynamic()) {
//			serializer.beginElement("RigidDynamic");
//
//			RigidDynamic& rigidDynamic = actor.getRigidDynamicCmp();
//			writeRigidDynamic(serializer, actor, rigidDynamic);
//
//			serializer.endElement();
//		}
//		if (actor.hasRigidStatic()) {
//			serializer.beginElement("RigidStatic");
//
//			RigidStatic& rigidStatic = actor.getRigidStaticCmp();
//			writeRigidStatic(serializer, actor, rigidStatic);
//
//			serializer.endElement();
//		}
//		if (actor.hasTransform()) {
//			serializer.beginElement("Transform");
//
//			Transform& transform = actor.getTransformCmp();
//			writeTransform(serializer, transform);
//
//			serializer.endElement();
//		}
//
//		serializer.endElement();
//		serializer.endSerialization();
//	}
}
