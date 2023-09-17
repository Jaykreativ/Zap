#include "Actor.h"
#include "glm/gtc/matrix_transform.hpp"

namespace Zap {
	Actor::Actor(){}
	Actor::~Actor(){}

	void Actor::translate(glm::vec3 vec) {
		m_transform = glm::translate(m_transform, vec);
	}
	void Actor::translate(float x, float y, float z) {
		translate({ x, y, z });
	}

	void Actor::setPos(float x, float y, float z) {
		setPos(glm::vec3(x, y, z));
	}
	void Actor::setPos(glm::vec3 pos) {
		translate(pos - glm::vec3(m_transform[3]));
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