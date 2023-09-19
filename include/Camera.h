#pragma once

#include "Zap.h"
#include "Actor.h"
#include "glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

namespace Zap {
	class Camera : public Actor
	{
	public:
		Camera();
		~Camera();

		glm::mat4 getView();

		glm::mat4 getPerspective(float aspect);
	};
}