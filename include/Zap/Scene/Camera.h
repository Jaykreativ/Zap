#pragma once

#include "Zap/Zap.h"
#include "Zap/Scene/Component.h"
#include "glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

namespace Zap {
	class Camera : public Component
	{
	public:
		Camera(Actor* pActor, glm::vec3 offset);
		Camera(Actor* pActor);

		glm::mat4 getView();

		glm::mat4 getPerspective(float aspect);
	private:
		glm::vec3 m_offset;

		static std::vector<Camera> all;

		friend class Actor;
		friend class Renderer;
		friend class PBRenderer;
	};
}