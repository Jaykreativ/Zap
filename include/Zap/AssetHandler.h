#pragma once

#include "Zap/UUID.h"
#include "Zap/Scene/Mesh.h"
#include "Zap/Scene/Material.h"

namespace Zap {
	class AssetHandler
	{
	public:
		AssetHandler();
		~AssetHandler();

		std::unordered_map<UUID, MeshData>::const_iterator beginMeshes() const;
		std::unordered_map<UUID, MeshData>::iterator beginMeshes();

		std::unordered_map<UUID, MeshData>::const_iterator endMeshes() const;
		std::unordered_map<UUID, MeshData>::iterator endMeshes();

		bool existsMeshData(UUID handle) const;

		const MeshData* getMeshData(UUID handle) const;

		MeshData* getMeshDataPtr(UUID handle);

		std::unordered_map<UUID, MaterialData>::const_iterator beginMaterials() const;
		std::unordered_map<UUID, MaterialData>::iterator beginMaterials();

		std::unordered_map<UUID, MaterialData>::const_iterator endMaterials() const;
		std::unordered_map<UUID, MaterialData>::iterator endMaterials();

		bool existsMaterialData(UUID handle) const;

		const MaterialData* getMaterialData(UUID handle) const;
		
		MaterialData* getMaterialDataPtr(UUID handle);

	private:
		std::unordered_map<UUID, MeshData> m_meshes = {};
		std::unordered_map<UUID, MaterialData> m_materials = {};

		friend class Base;
		friend class Mesh;
		friend class Material;
		friend class ModelLoader;
	};
}

