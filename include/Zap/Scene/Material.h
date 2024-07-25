#pragma once

#include "glm.hpp"

namespace Zap {
    class Material {
    public:
        Material() = default;
        ~Material() = default;

        alignas(16) glm::vec3 albedoColor = { 1, 1, 1};
        alignas(4) uint32_t albedoMap = 0xFFFFFFFF;
        alignas(4) float metallic = 0;
        alignas(4) uint32_t metallicMap = 0xFFFFFFFF;
        alignas(4) float roughness = 0.6;
        alignas(4) uint32_t roughnessMap = 0xFFFFFFFF;
        alignas(16) glm::vec4 emissive = { 0, 0, 0, 0 };
        alignas(4) uint32_t emissiveMap = 0xFFFFFFFF;
    };
}