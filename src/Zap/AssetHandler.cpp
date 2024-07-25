#include "Zap/AssetHandler.h"

namespace Zap {
	AssetHandler::AssetHandler() {

	}

	AssetHandler::~AssetHandler() {

	}

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

	const MeshData* AssetHandler::getMeshData(UUID handle) const {
		return &m_meshes.at(handle);
	}

	MeshData* AssetHandler::getMeshDataPtr(UUID handle) {
		return &m_meshes.at(handle);
	}
}
