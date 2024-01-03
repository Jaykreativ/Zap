#pragma once

#include "Zap/Zap.h"
#include "Zap/Scene/Mesh.h"

namespace Zap {
	struct Model {
		std::vector<Material> m_Materials;
		std::vector<uint32_t> m_meshes;
	};
}