#pragma once

#include "glm.hpp"
#include "Zap/UUID.h"
#include "Zap/Scene/Texture.h"

namespace Zap {
	struct MaterialGpuData {
		alignas(16) glm::vec4 albedoColor = { 1, 1, 1, 1 };
		alignas(4) uint32_t albedoMap = 0xFFFFFFFF;
		alignas(4) float metallic = 0;
		alignas(4) uint32_t metallicMap = 0xFFFFFFFF;
		alignas(4) float roughness = 0.5;
		alignas(4) uint32_t roughnessMap = 0xFFFFFFFF;
		alignas(16) glm::vec4 emissive = { 0, 0, 0, 0 };
		alignas(4) uint32_t emissiveMap = 0xFFFFFFFF;
	};

	struct MaterialData {
		glm::vec4 albedoColor = { 1, 1, 1, 1 };
		Texture albedoMap = (UUID)0;
		float metallic = 0;
		Texture metallicMap = (UUID)0;
		float roughness = 0.5;
		Texture roughnessMap = (UUID)0;
		glm::vec4 emissive = { 0, 0, 0, 0 };
		Texture emissiveMap = (UUID)0;
	};
	
	class Material {
	public:
		Material();
		Material(UUID handle);
		~Material();

		void destroy();

		bool exists() const;

		UUID getHandle();

		void setAlbedo(glm::vec3 albedo);
		void setAlbedo(glm::vec4 albedo);

		void setMetallic(float metallic);

		void setRoughness(float roughness);

		void setEmissive(glm::vec4 emissive);

		glm::vec4 getAlbedo();

		bool hasAlbedoMap();

		Texture getAlbedoMap();

		float getMetallic();

		bool hasMetallicMap();

		Texture getMetallicMap();

		float getRoughness();
		
		bool hasRoughnessMap();

		Texture getRoughnessMap();

		glm::vec3 getEmissive();

		float getEmissiveValue();

		bool hasEmissiveMap();

		Texture getEmissiveMap();

	private:
		UUID m_handle;
	};
}