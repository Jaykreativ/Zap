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

		bool raycast(glm::vec3 origin, glm::vec3 unitDir, uint32_t maxDistance, RaycastOutput* out, physx::PxQueryFilterCallback* filterCallback); // cleanup query filter

		void simulate(float elapsedTime);

	private:
		Scene(UUID handle);

		UUID m_dataHandle = UUID(0); // rework data structure without handles

		friend class Base;
		friend class RigidDynamicComponent;
		friend class RigidStaticComponent;
	};
}

