#include "Actor.h"
#include "glm/gtc/matrix_transform.hpp"

namespace Zap {
	Actor::Actor(){}
	Actor::~Actor(){}

	void Actor::setPos(float x, float y, float z) {
		setPos(glm::vec3(x, y, z));
	}
	void Actor::setPos(glm::vec3 pos) {
		m_transform = glm::translate(m_transform, pos - glm::vec3(m_transform[3]));
	}

	void Actor::setTransform(glm::mat4& transform) {
		m_transform = transform;
	}

	glm::vec3 Actor::getPos() {
		return glm::vec3(m_transform[3]);
	}

	glm::mat4 Actor::getTransform() {
		return m_transform;
	}
}