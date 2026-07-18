#pragma once

#include "Zap/Zap.h"
#include "Zap/AssetHandling/Asset.h"
#include "Zap/AssetHandling/AssetTypes/Mesh.h"
#include "Zap/AssetHandling/AssetTypes/Material.h"

namespace Zap {
	class Model {
	public:
		std::vector<AssetHandle<Mesh>> meshes;
		std::vector<AssetHandle<Material>> materials;
		std::vector<glm::mat4> transforms;
		glm::vec3 boundMin = { 0, 0, 0 };
		glm::vec3 boundMax = { 0, 0, 0 };
	};
}