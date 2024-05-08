#pragma once

#include "Zap/Zap.h"
#include "Zap/Scene/Mesh.h"

namespace Zap {
	struct Model {
		std::string filepath = "";
		std::vector<Material> materials;
		std::vector<uint32_t> meshes;
	};
}