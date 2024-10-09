#pragma once

#include "Zap/Zap.h"
#include "Zap/Scene/Mesh.h"
#include "Zap/Scene/Material.h"

namespace Zap {
	struct Model {
		std::string filepath = "";
		std::vector<Material> materials;
		std::vector<Mesh> meshes;
		glm::vec3 m_boundMin = { 0, 0, 0 };
		glm::vec3 m_boundMax = { 0, 0, 0 };
	};
}