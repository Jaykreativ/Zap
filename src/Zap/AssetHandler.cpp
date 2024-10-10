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

	const MaterialData* AssetHandler::getMaterialData(UUID handle) const {
		return &m_materials.at(handle);
	}

	MaterialData* AssetHandler::getMaterialDataPtr(UUID handle) {
		return &m_materials.at(handle);
	}
}
