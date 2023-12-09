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

		void lookAtCenter();

		void lookAtFront();

		void setOffset(glm::mat4 offset);

		glm::mat4 getOffset();

		glm::mat4 getView();

		glm::mat4 getPerspective(float aspect);
	private:
		bool m_lookAtCenter = false;
		glm::mat4 m_offset;

		static std::vector<Camera> all;

		
		friend class Actor;
		friend class Renderer;
		friend class PBRenderer;
	};
}