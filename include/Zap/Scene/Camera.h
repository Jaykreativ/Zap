#pragma once

#include "Zap/Zap.h"
#include "Zap/Scene/Actor.h"
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