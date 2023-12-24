#pragma once

#include "Zap/Zap.h"
#include "Zap/UUID.h"
#include "Actor.h"

#include <vector>

namespace Zap {

	struct SceneData {

	};

	class Base {
	private:
		std::unordered_map<UUID, SceneData> m_scenes;
	};

	class Scene
	{
	public:
		Scene();
		~Scene();

		uint32_t m_dataHandle;

		struct RaycastOutput {
			Actor* pActor;
			float distance;
			glm::vec3 normal;
			glm::vec3 position;
		};

		static bool raycast(glm::vec3 origin, glm::vec3 unitDir, uint32_t maxDistance, RaycastOutput* out, physx::PxQueryFilterCallback* filterCallback); // cleanup query filter

		static void simulate(float elapsedTime);
	};
}

