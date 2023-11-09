#include "Zap/Scene/Transform.h"
#include "glm/gtc/matrix_transform.hpp"

namespace Zap {
	std::vector<Transform> Transform::all;

	Transform::Transform(glm::mat4 transform, Actor* pActor)
		: Component(pActor), m_transform(transform)
	{
		m_id = all.size();
		all.push_back(*this);
	}

	void Transform::translate(glm::vec3 vec) {
		m_transform = glm::translate(m_transform, vec);
	}

	void Transform::translate(float x, float y, float z) {
		translate({ x, y, z });
	}

	void Transform::setPos(float x, float y, float z) {
		setPos(glm::vec3(x, y, z));
	}

	void Transform::setPos(glm::vec3 pos) {
		m_transform[3] = glm::vec4(pos, 1);
	}

	void Transform::rotateX(float angle) {
		rotate(angle, glm::vec3(1, 0, 0));
	}

	void Transform::rotateY(float angle) {
		rotate(angle, glm::vec3(0, 1, 0));
	}

	void Transform::rotateZ(float angle) {
		rotate(angle, glm::vec3(0, 0, 1));
	}

	void Transform::rotate(float angle, glm::vec3 axis) {
		m_transform = glm::rotate(m_transform, glm::radians<float>(angle), axis);
	}

	void Transform::setScale(glm::vec3 scale) {
		m_transform = glm::scale(m_transform, scale);
	}

	void Transform::setScale(float x, float y, float z) {
		setScale({ x, y, z });
	}

	void Transform::setTransform(glm::mat4& transform) {
		m_transform = transform;
	}

	glm::vec3 Transform::getPos() {
		return glm::vec3(m_transform[3]);
	}

	glm::mat4 Transform::getTransform() {
		return m_transform;
	}
}