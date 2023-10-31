#pragma once

#include "glm.hpp"

namespace Zap {
    class Material {
    public:
        Material() = default;
        ~Material() = default;

        glm::vec3 m_AlbedoColor = { 1, 1, 1 };
    };
}