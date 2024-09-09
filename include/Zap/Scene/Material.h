#pragma once

#include "glm.hpp"
#include "Zap/UUID.h"

namespace Zap {
	struct MaterialData {
		alignas(16) glm::vec4 albedoColor = { 1, 1, 1, 1 };
		alignas(4) uint32_t albedoMap = 0xFFFFFFFF;
		alignas(4) float metallic = 0;
		alignas(4) uint32_t metallicMap = 0xFFFFFFFF;
		alignas(4) float roughness = 0.5;
		alignas(4) uint32_t roughnessMap = 0xFFFFFFFF;
		alignas(16) glm::vec4 emissive = { 0, 0, 0, 0 };
		alignas(4) uint32_t emissiveMap = 0xFFFFFFFF;
	};
	
	class Material {
	public:
		Material();
		~Material();

		void destroy();

		UUID getHandle();

		void setAlbedo(glm::vec3 albedo);
		void setAlbedo(glm::vec4 albedo);

		void setMetallic(float metallic);

		void setRoughness(float roughness);

		void setEmissive(glm::vec4 emissive);

	private:
		UUID m_handle;
	};
}