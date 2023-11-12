#include "Zap/Scene/Actor.h"
#include "Zap/Scene/MeshComponent.h"
#include "Zap/Scene/PhysicsComponent.h"
#include "Zap/Scene/Light.h"
#include "Zap/Scene/Camera.h"
#include "Zap/Scene/Transform.h"
#include "glm/gtc/matrix_transform.hpp"

namespace Zap {
	Actor::Actor(){
		m_components.push_back(ComponentAccess{COMPONENT_TYPE_NONE, 0});
	}

	Actor::~Actor(){}
	
	bool Actor::addTransform(glm::mat4 transform) {
		if (m_transformState == TRANSFORM_STATE_NONE) {
			uint32_t id;
			id = Transform(transform, this).getID();
			m_components[0].id = id;
			m_components[0].type = COMPONENT_TYPE_TRANSFORM;
			m_transformState = TRANSFORM_STATE_COMPONENT;
			return true;
		}
		else {
			return false;
		}
	}

	bool Actor::addMesh(Mesh* pMesh) {
		uint32_t id = MeshComponent(this, pMesh).getID();
		m_components.push_back(ComponentAccess{ COMPONENT_TYPE_MESH, id });
		return true;
	}
	
	bool Actor::addPhysics(PhysicsType type, Shape shape) {
		uint32_t id = PhysicsComponent(type, shape, this).getID();
		m_components.push_back(ComponentAccess{ COMPONENT_TYPE_PHYSICS, id });
		return true;
	}

	bool Actor::addLight(glm::vec3 color) {
		uint32_t id;
		id = Light(this, color).getID();
		m_components.push_back(ComponentAccess{ COMPONENT_TYPE_LIGHT, id });
		return true;
	}

	bool Actor::addCamera(glm::vec3 offset) {
		uint32_t id;
		id = Camera(this, offset).getID();
		m_components.push_back(ComponentAccess{ COMPONENT_TYPE_CAMERA, id });
		return true;
	}

	glm::mat4 Actor::getTransform() {
		switch (m_transformState) {
		case TRANSFORM_STATE_NONE:
			return glm::mat4(1);
		case TRANSFORM_STATE_COMPONENT || TRANSFORM_STATE_PHYSICS:
			return Transform::all[m_components[0].id].m_transform;
		default:
			std::cerr << "Not a valid TransformState: " << m_transformState << "\n";
			throw std::runtime_error("Not a valid TransformState\n");
		}
	}

	void Actor::setTransform(glm::mat4 transform) {
		switch (m_transformState) {
		case TRANSFORM_STATE_NONE:
			return;
		case TRANSFORM_STATE_COMPONENT || TRANSFORM_STATE_PHYSICS:
			Transform::all[m_components[0].id].m_transform = transform;
			return;
		default:
			std::cerr << "Not a valid TransformState: " << m_transformState << "\n";
			throw std::runtime_error("Not a valid TransformState\n");
		}
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

	Transform* Actor::getTransformComponent() {
		switch (m_transformState) {
		case TRANSFORM_STATE_NONE:
			return nullptr;
		case TRANSFORM_STATE_COMPONENT || TRANSFORM_STATE_PHYSICS:
			return &Transform::all[m_components[0].id];
		default:
			std::cerr << "Not a valid TransformState: " << m_transformState << "\n";
			throw std::runtime_error("Not a valid TransformState\n");
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

	PhysicsComponent* Actor::getPhysicsComponent(uint32_t index) {
		int num = 0;
		for (ComponentAccess cA : m_components) {
			if (cA.type == COMPONENT_TYPE_PHYSICS) {
				if (num >= index) {
					return &PhysicsComponent::all[cA.id];
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
}