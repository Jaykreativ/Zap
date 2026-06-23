#include "Zap/AssetHandling/AssetTypes/Material.h"
#include "Zap/Zap.h"

namespace Zap {
	Material::Material() {}
	Material::~Material() {}

	void Material::setAlbedo(glm::vec3 albedo) {
		setAlbedo(glm::vec4(albedo, 1.0));
	}

	void Material::setAlbedo(glm::vec4 albedo) {
		m_albedoColor = albedo;
	}

	void Material::setMetallic(float metallic) {
		m_metallic = metallic;
	}

	void Material::setRoughness(float roughness) {
		m_roughness = roughness;
	}

	void Material::setEmissive(glm::vec4 emissive) {
		m_emissive = emissive;
	}

	glm::vec4 Material::getAlbedo() {
		return m_albedoColor;
	}

	bool Material::hasAlbedoMap() {
		return m_albedoMap;
	}

	AssetHandle<Texture> Material::getAlbedoMap() {
		ZP_WARN(m_albedoMap, "Invalid albedoMap requested");
		return m_albedoMap;
	}

	float Material::getMetallic() {
		return m_metallic;
	}

	bool Material::hasMetallicMap() {
		return m_metallicMap;
	}

	AssetHandle<Texture> Material::getMetallicMap() {
		ZP_WARN(m_metallicMap, "Invalid metallicMap requested");
		return m_metallicMap;
	}

	float Material::getRoughness() {
		return m_roughness;
	}

	bool Material::hasRoughnessMap() {
		return m_roughnessMap;
	}

	AssetHandle<Texture> Material::getRoughnessMap() {
		ZP_WARN(m_roughnessMap, "Invalid roughnessMap requested");
		return m_roughnessMap;
	}

	glm::vec3 Material::getEmissive() {
		return m_emissive;
	}

	float Material::getEmissiveValue() {
		return m_emissive.w;
	}

	bool Material::hasEmissiveMap() {
		return m_emissiveMap;
	}

	AssetHandle<Texture> Material::getEmissiveMap() {
		ZP_WARN(m_emissiveMap, "Invalid emissiveMap requested");
		return m_emissiveMap;
	}
}