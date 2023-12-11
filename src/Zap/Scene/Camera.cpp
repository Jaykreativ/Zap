#include "Zap/Scene/Camera.h"
#include "Zap/Scene/Actor.h"

namespace Zap {
	std::vector<Camera> Camera::all;

	Camera::Camera(Actor* pActor, glm::vec3 offset)
		: Component(pActor)
	{
		m_offset = glm::mat4(1);
		m_offset[3] = glm::vec4(offset, 1);

		m_id = all.size();
		all.push_back(*this);
	}
	Camera::Camera(Actor* pActor)
		: Camera(pActor, glm::vec3(0, 0, 0))
	{}

	void Camera::lookAtCenter() {
		m_lookAtCenter = true;
	}

	void Camera::lookAtFront() {
		m_lookAtCenter = false;
	}

	void Camera::setOffset(glm::mat4 offset) {
		m_offset = offset;
	}

	glm::mat4 Camera::getOffset() {
		return m_offset;
	}

	glm::mat4 Camera::getView() {
		auto transform = m_pActor->getTransform();
		if (m_lookAtCenter) {
			return glm::lookAt(glm::vec3(transform[3]) + glm::vec3(m_offset[3]), glm::vec3(transform[3]), glm::vec3(m_offset[1]));
		}
		else {
			return glm::lookAt(glm::vec3(transform[3]) + glm::vec3(m_offset[3]), glm::vec3(transform[3]) + glm::vec3(m_offset[3]) + glm::vec3(transform[2]), glm::vec3(m_offset*transform[1]));
		}
	}

	glm::mat4 Camera::getPerspective(float aspect) {
		return glm::perspective<float>(glm::radians<float>(60), aspect, 0.01, 1000);
	}
}