#include "Zap/AssetHandling/AssetHandler.h"

#include "Zap/Serializer.h"
#include "Zap/AssetHandling/Loaders.h"

#include <fstream>
#include <filesystem>
#include <string>

namespace Zap {
	AssetHandler::AssetHandler() {}

	AssetHandler::~AssetHandler() {}

	bool AssetHandler::isMesh(UUID handle) const {
		return m_meshMap.count(handle);
	}
	bool AssetHandler::isMaterial(UUID handle) const {
		return m_materialMap.count(handle);
	}
	bool AssetHandler::isTexture(UUID handle) const {
		return m_textureMap.count(handle);
	}

	bool AssetHandler::isAsset(UUID handle) const {
		if (isMesh(handle)) return true;
		if (isMaterial(handle)) return true;
		if (isTexture(handle)) return true;
		return false;
	}

	AssetHandlerEventHandler& AssetHandler::getEventHandler() {
		return m_eventHandler;
	}

	FileLinker& AssetHandler::getFileLinker() {
		return m_fileLinker;
	}

	template<>
	std::unordered_map<UUID, std::unique_ptr<Mesh>>& AssetHandler::getMap() {
		return m_meshMap;
	}
	template<>
	std::unordered_map<UUID, std::unique_ptr<Material>>& AssetHandler::getMap() {
		return m_materialMap;
	}
	template<>
	std::unordered_map<UUID, std::unique_ptr<Texture>>& AssetHandler::getMap() {
		return m_textureMap;
	}
}
