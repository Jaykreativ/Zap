#pragma once

#include "Zap/Zap.h"
#include "Zap/AssetHandling/Asset.h"
#include "Zap/AssetHandling/AssetTypes/Mesh.h"
#include "Zap/AssetHandling/AssetTypes/Material.h"

namespace Zap {
	struct Model {
		std::vector<AssetHandle<Material>> materials;
		std::vector<AssetHandle<Mesh>> meshes;
		glm::vec3 boundMin = { 0, 0, 0 };
		glm::vec3 boundMax = { 0, 0, 0 };
	};
}