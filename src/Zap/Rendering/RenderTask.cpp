#include "Zap/Rendering/RenderTask.h"

#include "Zap/Rendering/Renderer.h"
#include "Zap/Scene/Scene.h"
#include "Zap/Scene/Actor.h"
#include "Zap/Scene/Model.h"
#include "Zap/Scene/Mesh.h"

namespace Zap {
	RenderTask::RenderTask() {}

	RenderTask::RenderTask(Scene* pScene)
		: m_pScene(pScene)
	{}

	RenderTask::~RenderTask(){}

	std::unordered_map<UUID, Model>::iterator RenderTask::beginSceneModels() {
		return m_pScene->m_modelComponents.begin();
	}

	std::unordered_map<UUID, Model>::iterator RenderTask::endSceneModels() {
		return m_pScene->m_modelComponents.end();
	}

	vk::Registery* RenderTask::getRegistery() {
		return &Base::getBase()->m_registery;
	}

	vk::Buffer* RenderTask::getScenePerMeshInstanceBuffer() {
		if (m_pScene) {
			return &m_pScene->m_perMeshInstanceBuffer;
		}
		return nullptr;
	}

	vk::Buffer* RenderTask::getSceneLightBuffer() {
		if (m_pScene) {
			return &m_pScene->m_lightBuffer;
		}
		return nullptr;
	}

	Model* RenderTask::getActorModel(Actor actor) {
		return &m_pScene->m_modelComponents.at(actor);
	}

	uint32_t RenderTask::getMeshInstanceIndex(UUID actor, Mesh mesh) {
		if (m_pScene->m_meshInstanceIndices.count(mesh.getHandle() + (UUID)actor)) {
			return m_pScene->m_meshInstanceIndices.at(mesh.getHandle() + (UUID)actor);
		}
		return 0;
	}
	uint32_t RenderTask::getMeshInstanceIndex(Actor actor, Mesh mesh) {
		return getMeshInstanceIndex((UUID)actor, mesh);
	}

	std::unordered_map<UUID, TextureData>* RenderTask::getTextureDataMap() {
		return &Base::getBase()->m_assetHandler.m_textures;
	}

	uint32_t RenderTask::getTextureIndex(UUID texture) {
		return Base::getBase()->m_textureIndices.at(texture);
	}

	vk::Sampler* RenderTask::getTextureSampler() {
		return &Base::getBase()->m_textureSampler;
	}

}