#pragma once

#include "glm.hpp"

namespace Zap {
	struct Light
	{
		glm::vec3 color = {1, 1, 1};
		float strength = 1;
		float radius = 1;
	};
}