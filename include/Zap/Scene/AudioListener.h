#pragma once

#include "glm/glm.hpp"

#include <stdint.h> 

namespace Zap {
	class AudioListener
	{
	public:
		AudioListener();
		~AudioListener();

		glm::vec3 lastPos = { 0, 0, 0 };
	};
}

