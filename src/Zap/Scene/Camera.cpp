#include "Zap/Scene/Camera.h"
#include "Zap/Scene/Actor.h"

namespace Zap {
	std::vector<Camera> Camera::all;

	Camera::Camera(Actor* pActor, glm::vec3 offset)
		: Component(pActor), m_offset(offset)
	{
		m_id = all.size();
		all.push_back(*this);
	}
	Camera::Camera(Actor* pActor)
		: Camera(pActor, glm::vec3(0, 0, 0))
	{}

	glm::mat4 Camera::getView() {
		auto transform = m_pActor->m_transform;
		return glm::lookAt(glm::vec3(transform[3]), glm::vec3(transform[3]) + glm::vec3(transform[2]), glm::vec3(transform[1]));
	}

	glm::mat4 Camera::getPerspective(float aspect) {
		return glm::perspective<float>(glm::radians<float>(60), aspect, 0.01, 1000);
	}
}