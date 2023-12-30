#pragma once

#include "glm.hpp"

namespace Zap {
    struct Camera {
        bool lookAtCenter = false;
        glm::mat4 offset;
    };
}