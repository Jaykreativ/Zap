#include "Camera.h"

namespace Zap {
	Camera::Camera(){}
	Camera::~Camera(){}

	glm::mat4 Camera::getView() {
		return glm::lookAt(glm::vec3(m_transform[3]), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
	}

	glm::mat4 Camera::getPerspective(float aspect) {
		return glm::perspective<float>(glm::radians<float>(60), 1, 0.01, 1000);
	}
}