#pragma once

#include "Zap/Zap.h"
#include "Zap/Scene/Mesh.h"
#include "Zap/Scene/Material.h"

namespace Zap {
	struct Model {
		std::string filepath = "";
		std::vector<Material> materials;
		std::vector<Mesh> meshes;
		glm::vec3 boundMin = { 0, 0, 0 };
		glm::vec3 boundMax = { 0, 0, 0 };
	};
}