#pragma once

#include "Actor.h"

#include <vector>

namespace Zap {
    class Scene
    {
    public:
        Scene();
        ~Scene();

        static void simulate(float elapsedTime);
    };
}

