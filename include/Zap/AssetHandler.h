#pragma once

#include "Zap/UUID.h"
#include "Zap/Scene/Mesh.h"

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

	private:
		std::unordered_map<UUID, MeshData> m_meshes = {};

		friend class Base;
		friend class Mesh;
		friend class ModelLoader;
	};
}

