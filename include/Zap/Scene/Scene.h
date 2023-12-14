#pragma once

#include "Zap/Zap.h"
#include "Actor.h"

#include <vector>

namespace Zap {
    class Scene
    {
    public:
        Scene();
        ~Scene();

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

