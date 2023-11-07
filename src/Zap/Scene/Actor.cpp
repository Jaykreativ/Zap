#include "Zap/Scene/Actor.h"
#include "Zap/Scene/MeshComponent.h"
#include "Zap/Scene/Light.h"
#include "Zap/Scene/Camera.h"
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

	void Actor::setScale(glm::vec3 scale) {
		m_transform = glm::scale(m_transform, scale);
	}
	void Actor::setScale(float x, float y, float z) {
		setScale({ x, y, z });
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

	std::vector<uint32_t> Actor::getComponentIDs(ComponentType type) {
		std::vector<uint32_t> components;
		for (ComponentAccess component : m_components) {
			if (component.type == type) components.push_back(component.id);
		}
		return components;
	}

	Component* Actor::getComponent(ComponentType type, uint32_t index) {
		uint32_t num = 0;
		for (ComponentAccess component : m_components) {
			if (component.type == type) {
				if (num >= index) {
					switch (type) {
					case COMPONENT_TYPE_CAMERA:
						return &Camera::all[component.id];
					case COMPONENT_TYPE_LIGHT:
						return &Light::all[component.id];
					case COMPONENT_TYPE_MESH:
						return &MeshComponent::all[component.id];
					case COMPONENT_TYPE_NONE:
						return nullptr;
					default:
						std::cerr << "No return case implemented for this ComponentType\n";
						throw std::runtime_error("No return case implemented for this ComponentType\n");
					}
				}
				num++;
			}
		}
	}

	MeshComponent* Actor::getMeshComponent(uint32_t index) {
		int num = 0;
		for (ComponentAccess cA : m_components) {
			if (cA.type == COMPONENT_TYPE_MESH) {
				if (num >= index) {
					return &MeshComponent::all[cA.id];
				}
				num++;
			}
		}
	}

	Light* Actor::getLightComponent(uint32_t index) {
		int num = 0;
		for (ComponentAccess cA : m_components) {
			if (cA.type == COMPONENT_TYPE_LIGHT) {
				if (num >= index) {
					return &Light::all[cA.id];
				}
				num++;
			}
		}
	}

	Camera* Actor::getCameraComponent(uint32_t index) {
		int num = 0;
		for (ComponentAccess cA : m_components) {
			if (cA.type == COMPONENT_TYPE_CAMERA) {
				if (num >= index) {
					return &Camera::all[cA.id];
				}
				num++;
			}
		}
	}

	void Actor::addMeshComponent(Mesh* pMesh) {
		uint32_t id;
		id = MeshComponent(this, pMesh).getID();
		m_components.push_back(ComponentAccess{ COMPONENT_TYPE_MESH, id });
	}

	void Actor::addLightComponent(glm::vec3 color) {
		uint32_t id;
		id = Light(this, color).getID();
		m_components.push_back(ComponentAccess{ COMPONENT_TYPE_LIGHT, id });
	}

	void Actor::addCameraComponent(glm::vec3 offset) {
		uint32_t id;
		id = Camera(this, offset).getID();
		m_components.push_back(ComponentAccess{ COMPONENT_TYPE_CAMERA, id });
	}
}