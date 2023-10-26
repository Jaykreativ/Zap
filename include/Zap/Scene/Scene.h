#pragma once

#include "Actor.h"

#include <vector>

namespace Zap {
    class Scene
    {
    public:
        Scene();
        ~Scene();

        void addActor(Actor& actor);

    private:
        std::vector<Actor*> m_physicsActors;
    };
}

