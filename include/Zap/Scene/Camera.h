#pragma once

#include "Zap/Zap.h"
#include "glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

namespace Zap {
    struct Camera {
        bool lookAtCenter = false;
        glm::mat4 offset;
    };
}