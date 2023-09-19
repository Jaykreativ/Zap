#include "Camera.h"

namespace Zap {
	Camera::Camera(){}
	Camera::~Camera(){}

	glm::mat4 Camera::getView() {
		return glm::lookAt(glm::vec3(m_transform[3]), glm::vec3(m_transform[3]) + glm::vec3(m_transform[2]), glm::vec3(m_transform[1]));
	}

	glm::mat4 Camera::getPerspective(float aspect) {
		return glm::perspective<float>(glm::radians<float>(60), aspect, 0.01, 1000);
	}
}