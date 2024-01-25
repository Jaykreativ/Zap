#pragma once

#include "Zap/Zap.h"
#include "Zap/UUID.h"
#include "Zap/Scene/Actor.h"
#include "Zap/Scene/Camera.h"
#include "Zap/Scene/Light.h"
#include "Zap/Scene/Model.h"
#include "Zap/Scene/PhysicsComponent.h"
#include "Zap/Scene/Transform.h"

#include "glm.hpp"

#include <vector>

namespace Zap {

	class Actor; // forward declaration

	class Scene
	{
	public:
		Scene();
		~Scene();

		struct RaycastOutput {
			Actor actor;
			float distance;
			glm::vec3 normal;
			glm::vec3 position;
		};

		void init();

		void destroy();

		void attachActor(Actor& actor);

		bool raycast(glm::vec3 origin, glm::vec3 unitDir, uint32_t maxDistance, RaycastOutput* out, physx::PxQueryFilterCallback* filterCallback); // cleanup query filter

		void simulate(float elapsedTime);

	private:
		physx::PxScene* m_pxScene;

		//TODO add parent/child system to actors
#ifdef ZP_ENTITY_COMPONENT_SYSTEM_ACCESS
	public:
#endif
		std::unordered_map<UUID, Camera>                   m_cameraComponents;// TODO rework access system
		std::unordered_map<UUID, Light>                    m_lightComponents;
		std::unordered_map<UUID, Model>                    m_modelComponents;
		std::vector<uint32_t>                              m_meshReferences;
		std::unordered_map<UUID, RigidDynamicComponent>    m_rigidDynamicComponents;
		std::unordered_map<UUID, RigidStaticComponent>     m_rigidStaticComponents;
		std::unordered_map<UUID, Transform>                m_transformComponents;
#ifdef ZP_ENTITY_COMPONENT_SYSTEM_ACCESS
	private:
#endif

		friend class Base;
		friend class Actor;
		friend class RigidDynamicComponent;
		friend class RigidStaticComponent;
		friend class PBRenderer;
		friend class RaytracingRenderer;
	};
}

