#include "Zap/AssetHandler.h"

#include "Zap/Serializer.h"
#include "Zap/FileLoader.h"

#include <fstream>
#include <string>

namespace Zap {
	AssetHandler::AssetHandler() {}

	AssetHandler::~AssetHandler() {}

	/* Mesh */

	std::unordered_map<UUID, MeshData>::const_iterator AssetHandler::beginMeshes() const {
		return m_meshes.begin();
	}

	std::unordered_map<UUID, MeshData>::iterator AssetHandler::beginMeshes() {
		return m_meshes.begin();
	}

	std::unordered_map<UUID, MeshData>::const_iterator AssetHandler::endMeshes() const {
		return m_meshes.end();
	}

	std::unordered_map<UUID, MeshData>::iterator AssetHandler::endMeshes() {
		return m_meshes.end();
	}

	bool AssetHandler::existsMeshData(UUID handle) const {
		return m_meshes.count(handle);
	}

	const MeshData& AssetHandler::getMeshData(UUID handle) const {
		return m_meshes.at(handle);
	}

	MeshData* AssetHandler::getMeshDataPtr(UUID handle) {
		return &m_meshes.at(handle);
	}

	/* Material */

	std::unordered_map<UUID, MaterialData>::const_iterator AssetHandler::beginMaterials() const {
		return m_materials.begin();
	}

	std::unordered_map<UUID, MaterialData>::iterator AssetHandler::beginMaterials() {
		return m_materials.begin();
	}

	std::unordered_map<UUID, MaterialData>::const_iterator AssetHandler::endMaterials() const {
		return m_materials.end();
	}

	std::unordered_map<UUID, MaterialData>::iterator AssetHandler::endMaterials() {
		return m_materials.end();
	}

	bool AssetHandler::existsMaterialData(UUID handle) const {
		return m_materials.count(handle);
	}

	const MaterialData& AssetHandler::getMaterialData(UUID handle) const {
		return m_materials.at(handle);
	}

	MaterialData* AssetHandler::getMaterialDataPtr(UUID handle) {
		return &m_materials.at(handle);
	}

	/* Texture */

	std::unordered_map<UUID, TextureData>::const_iterator AssetHandler::beginTextures() const {
		return m_textures.begin();
	}

	std::unordered_map<UUID, TextureData>::iterator AssetHandler::beginTextures() {
		return m_textures.begin();
	}

	std::unordered_map<UUID, TextureData>::const_iterator AssetHandler::endTextures() const {
		return m_textures.end();
	}

	std::unordered_map<UUID, TextureData>::iterator AssetHandler::endTextures() {
		return m_textures.end();
	}

	bool AssetHandler::existsTextureData(UUID handle) const {
		return m_textures.count(handle);
	}

	const TextureData& AssetHandler::getTextureData(UUID handle) const {
		return m_textures.at(handle);
	}

	TextureData* AssetHandler::getTextureDataPtr(UUID handle) {
		return &m_textures.at(handle);
	}

	/* Load / Save */

	void AssetHandler::loadFromFile(std::string filepath) {
		destroyAssets();

		Serializer serializer;
		serializer.beginDeserialization(filepath.c_str());

		bool existsData = true;
		int meshCount = serializer.readAttributei("meshCount", &existsData);
		int materialCount = serializer.readAttributei("materialCount", &existsData);
		int textureCount = serializer.readAttributei("textureCount", &existsData);
		if (!existsData) return;

		for (int i = 0; i < meshCount; i++) {
			if (serializer.beginElement("Mesh" + std::to_string(i))) {
				std::cout << "Mesh -> " << i << " (AssetHandler)\n";
				UUID handle = serializer.readAttributeUUID("handle");
				std::string filepath = serializer.readAttribute("filepath");
				int index = std::stoi(serializer.readAttribute("index"));
				glm::mat4 transform = serializer.readAttributeMat4("transform");

				MeshLoader meshloader;
				meshloader.loadFromFile(filepath, index, transform, handle);
				serializer.endElement();
			}
			else {
				std::cerr << "Failed to load Mesh" << i << " from zap asset library\n";
			}
		}
		for (int i = 0; i < materialCount; i++){
			if (serializer.beginElement("Material" + std::to_string(i))) {
				std::cout << "Material -> " << i << " (AssetHandler)\n";
				Material material = Material(serializer.readAttributeUUID("handle"));
				m_materials[material.getHandle()] = MaterialData{};

				material.setAlbedo(serializer.readAttributeVec4("albedo"));
				if(serializer.existsAttribute("albedoMap"))
					m_materials[material.getHandle()].albedoMap = serializer.readAttributeUUID("albedoMap");
				material.setMetallic(serializer.readAttributef("metallic"));
				if (serializer.existsAttribute("metallicMap"))
					m_materials[material.getHandle()].metallicMap = serializer.readAttributeUUID("metallicMap");
				material.setRoughness(serializer.readAttributef("roughness"));
				if (serializer.existsAttribute("roughnessMap"))
					m_materials[material.getHandle()].roughnessMap = serializer.readAttributeUUID("roughnessMap");
				material.setEmissive(serializer.readAttributeVec4("emissive"));
				if (serializer.existsAttribute("emissiveMap"))
					m_materials[material.getHandle()].emissiveMap = serializer.readAttributeUUID("emissiveMap");

				std::string path = serializer.readAttribute("filepath");
				int index = serializer.readAttributei("index");

				m_loadedMaterials.push_back(material);
				m_materialPaths[material.getHandle()] = {path, index};
				m_pathMaterialMap[{ path, index }] = material.getHandle();

				serializer.endElement();
			}
			else {
				std::cerr << "Failed to load Material" << i << " from zap asset library\n";
			}
		}
		for (int i = 0; i < textureCount; i++){
			if (serializer.beginElement("Texture" + std::to_string(i))) {
				std::cout << "Texture -> " << i << " (AssetHandler)\n";
				TextureLoader textureLoader = TextureLoader();
				UUID handle = serializer.readAttributeUUID("handle");
				// check for embedded textures
				if (serializer.existsAttribute("embedded")) {// embedded
					std::string textureID = serializer.readAttribute("filepath");
					std::string modelpath = serializer.readAttribute("embedded");
					textureLoader.load(modelpath, textureID, handle);
				}
				else {// file
					std::string filepath = serializer.readAttribute("filepath");
					textureLoader.load(filepath, handle);
				}
				serializer.endElement();
			}
			else {
				std::cerr << "Failed to load Texture" << i << " from zap asset library\n";
			}
		}

		serializer.endDeserialization();
	}

	void AssetHandler::saveToFile(std::filesystem::path filepath) {
		saveToFile(filepath.string());
	}
	void AssetHandler::saveToFile(std::string filepath) {
		Serializer serializer;
		serializer.beginSerialization(filepath.c_str());

		uint32_t i = 0;
		serializer.writeAttribute("meshCount", std::to_string(m_loadedMeshes.size()));
		for (Mesh mesh : m_loadedMeshes) {
			serializer.beginElement("Mesh" + std::to_string(i));
			serializer.writeAttribute("handle", std::to_string(mesh.getHandle()));
			serializer.writeAttribute("filepath", m_meshPaths[mesh.getHandle()].first);
			serializer.writeAttribute("index", std::to_string(m_meshPaths[mesh.getHandle()].second));
			serializer.writeAttribute("transform", *mesh.getTransform());
			serializer.endElement();
			i++;
		}
		i = 0;
		serializer.writeAttribute("materialCount", std::to_string(m_loadedMaterials.size()));
		for (Material material : m_loadedMaterials) {
			serializer.beginElement("Material" + std::to_string(i));
			serializer.writeAttribute("handle", std::to_string(material.getHandle()));
			serializer.writeAttribute("albedo", material.getAlbedo());
			if (material.hasAlbedoMap())
				serializer.writeAttribute("albedoMap", material.getAlbedoMap().getHandle());
			serializer.writeAttribute("metallic", material.getMetallic());
			if (material.hasMetallicMap())
				serializer.writeAttribute("metallicMap", material.getMetallicMap().getHandle());
			serializer.writeAttribute("roughness", material.getRoughness());
			if (material.hasRoughnessMap())
				serializer.writeAttribute("roughnessMap", material.getRoughnessMap().getHandle());
			serializer.writeAttribute("emissive", { material.getEmissive(), material.getEmissiveValue() });
			if (material.hasEmissiveMap())
				serializer.writeAttribute("emissiveMap", material.getEmissiveMap().getHandle());
			serializer.writeAttribute("filepath", m_materialPaths[material.getHandle()].first);
			serializer.writeAttribute("index", std::to_string(m_materialPaths[material.getHandle()].second));
			serializer.endElement();
			i++;
		}
		i = 0;
		serializer.writeAttribute("textureCount", std::to_string(m_loadedTextures.size()));
		for (Texture texture : m_loadedTextures) {
			serializer.beginElement("Texture" + std::to_string(i));
			serializer.writeAttribute("handle", std::to_string(texture.getHandle()));
			serializer.writeAttribute("filepath", m_texturePaths[texture.getHandle()].first);
			if (m_texturePaths[texture.getHandle()].second != "")
				serializer.writeAttribute("embedded", m_texturePaths[texture.getHandle()].second);
			serializer.endElement();
			i++;
		}

		serializer.endSerialization();
	}

	void AssetHandler::destroyAssets(){
#ifdef _DEBUG
		std::cout << "Destroying all assets, this will invalidate all actors using any assets\n";
#endif
		// destroy all resources
		for (auto& meshPair : m_meshes) {
			Mesh::destroy(&meshPair.second); // destroy mesh data
		}
		for (auto& materialPair : m_materials) {
			Material::destroy(&materialPair.second); // destroy material data
		}
		for (auto& texturePair : m_textures) {
			Texture::destroy(&texturePair.second); // destroy texture data
		}

		// clear AssetHandler
		m_meshes = {};
		m_loadedMeshes = {};
		m_meshPaths = {};
		m_pathMeshMap = {};

		m_materials = {};
		m_loadedMaterials = {};
		m_materialPaths = {};
		m_pathMaterialMap = {};

		m_textures = {};
		m_loadedTextures = {};
		m_texturePaths = {};
	}

	EventHandler<TextureLoadEvent>* AssetHandler::getTextureLoadEventHandler() {
		return &m_textureLoadEventHandler;
	}

	void AssetHandler::addTexture(Texture texture) {
		m_textures[texture.getHandle()] = TextureData{};
	}

	void AssetHandler::addLoadedTexture(Texture texture) {
		m_loadedTextures.push_back(texture);
		m_textureLoadEventHandler.pushEvent(TextureLoadEvent(texture));
	}
}
