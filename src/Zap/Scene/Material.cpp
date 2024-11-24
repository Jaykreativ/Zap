#include "Zap/Scene/Material.h"
#include "Zap/Zap.h"
#include "Zap/AssetHandler.h"

namespace Zap {
	Material::Material()
		: m_handle()
	{
		Base::getBase()->m_assetHandler.m_materials[m_handle] = MaterialData{};
	}

	Material::Material(UUID handle)
		: m_handle(handle)
	{}

	Material::~Material() {}

	void Material::destroy() {}

	bool Material::exists() const {
		return Base::getBase()->m_assetHandler.m_materials.count(m_handle);
	}

	UUID Material::getHandle() {
		return m_handle;
	}

	void Material::setAlbedo(glm::vec3 albedo) {
		setAlbedo(glm::vec4(albedo, 1.0));
	}

	void Material::setAlbedo(glm::vec4 albedo) {
		auto* base = Base::getBase();
		auto* data = base->m_assetHandler.getMaterialDataPtr(m_handle);
		data->albedoColor = albedo;
	}

	void Material::setMetallic(float metallic) {
		auto* base = Base::getBase();
		auto* data = base->m_assetHandler.getMaterialDataPtr(m_handle);
		data->metallic = metallic;
	}

	void Material::setRoughness(float roughness) {
		auto* base = Base::getBase();
		auto* data = base->m_assetHandler.getMaterialDataPtr(m_handle);
		data->roughness = roughness;
	}

	void Material::setEmissive(glm::vec4 emissive) {
		auto* base = Base::getBase();
		auto* data = base->m_assetHandler.getMaterialDataPtr(m_handle);
		data->emissive = emissive;
	}

	glm::vec4 Material::getAlbedo() {
		auto* base = Base::getBase();
		auto* data = base->m_assetHandler.getMaterialDataPtr(m_handle);
		return data->albedoColor;
	}

	float Material::getMetallic() {
		auto* base = Base::getBase();
		auto* data = base->m_assetHandler.getMaterialDataPtr(m_handle);
		return data->metallic;
	}

	float Material::getRoughness() {
		auto* base = Base::getBase();
		auto* data = base->m_assetHandler.getMaterialDataPtr(m_handle);
		return data->roughness;
	}

	glm::vec3 Material::getEmissive() {
		auto* base = Base::getBase();
		auto* data = base->m_assetHandler.getMaterialDataPtr(m_handle);
		return data->emissive;
	}

	float Material::getEmissiveValue() {
		auto* base = Base::getBase();
		auto* data = base->m_assetHandler.getMaterialDataPtr(m_handle);
		return data->emissive.w;
	}

}