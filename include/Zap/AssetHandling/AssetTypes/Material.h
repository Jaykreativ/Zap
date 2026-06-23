#pragma once

#include "glm.hpp"
#include "Zap/UUID.h"
#include "Zap/AssetHandling/Asset.h"
#include "Zap/AssetHandling/AssetTypes/Texture.h"

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
	
	class Material {
		friend class MaterialLoader;
	public:
		Material();
		~Material();

		void setAlbedo(glm::vec3 albedo);
		void setAlbedo(glm::vec4 albedo);

		void setMetallic(float metallic);

		void setRoughness(float roughness);

		void setEmissive(glm::vec4 emissive);

		glm::vec4 getAlbedo();

		bool hasAlbedoMap();

		AssetHandle<Texture> getAlbedoMap();

		float getMetallic();

		bool hasMetallicMap();

		AssetHandle<Texture> getMetallicMap();

		float getRoughness();
		
		bool hasRoughnessMap();

		AssetHandle<Texture> getRoughnessMap();

		glm::vec3 getEmissive();

		float getEmissiveValue();

		bool hasEmissiveMap();

		AssetHandle<Texture> getEmissiveMap();

	private:
		glm::vec4            m_albedoColor = { 1, 1, 1, 1 };
		AssetHandle<Texture> m_albedoMap;
		float                m_metallic = 0;
		AssetHandle<Texture> m_metallicMap;
		float                m_roughness = 0.5;
		AssetHandle<Texture> m_roughnessMap;
		glm::vec4            m_emissive = { 0, 0, 0, 0 };
		AssetHandle<Texture> m_emissiveMap;
	};
}