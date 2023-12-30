#pragma once

#include "Zap/Zap.h"
#include "Zap/UUID.h"
#include "glm.hpp"

#include <vector>

namespace Zap {

	class Actor; // forward declaration

	class Scene
	{
	public:
		Scene();// generates null handle
		~Scene();

		struct RaycastOutput {
			Actor* pActor;
			float distance;
			glm::vec3 normal;
			glm::vec3 position;
		};

		void init();

		void attachActor(Actor actor);

		bool raycast(glm::vec3 origin, glm::vec3 unitDir, uint32_t maxDistance, RaycastOutput* out, physx::PxQueryFilterCallback* filterCallback); // cleanup query filter

		void simulate(float elapsedTime);

	private:
		physx::PxScene* m_pxScene;

		//TODO add parent/child system to actors

		std::unordered_map<UUID, Camera> m_cameraComponents;
		std::unordered_map<UUID, Light> m_lightComponents;
		std::unordered_map<UUID, MeshComponent> m_meshComponents;
		std::unordered_map<UUID, RigidDynamicComponent> m_rigidDynamicComponents;
		std::unordered_map<UUID, RigidStaticComponent> m_rigidStaticComponents;
		std::unordered_map<UUID, Transform> m_transformComponents;

		friend class Base;
		friend class Actor;
		friend class RigidDynamicComponent;
		friend class RigidStaticComponent;
	};
}

