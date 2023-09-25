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
		m_transform[3] = glm::vec4(pos, 1);
	}

	void Actor::rotateX(float angle) {
		rotate(angle, glm::vec3(1, 0, 0));
	}
	void Actor::rotateY(float angle) {
		rotate(angle, glm::vec3(0, 1, 0));
	}
	void Actor::rotateZ(float angle) {
		rotate(angle, glm::vec3(0, 0, 1));
	}
	void Actor::rotate(float angle, glm::vec3 axis) {
		m_transform = glm::rotate(m_transform, glm::radians<float>(angle), axis);
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