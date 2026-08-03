#include "Zap/Rendering/RenderObjects/RenderTask.h"

#include "Zap/Rendering/Renderer.h"
#include "Zap/Scene/Scene.h"
#include "Zap/Scene/Actor.h"
#include "Zap/Scene/Components.h"
#include "Zap/AssetHandling/AssetTypes/Mesh.h"

namespace Zap {
	RenderTask::RenderTask(Renderer* pRenderer)
		: RenderObject(pRenderer)
	{}

	RenderTask::RenderTask(Renderer* pRenderer, Scene* pScene)
		: RenderObject(pRenderer), m_pScene(pScene)
	{}

	RenderTask::~RenderTask(){}

	std::unordered_map<UUID, Model>::iterator RenderTask::beginSceneModels() {
		return m_pScene->m_modelComponents.begin();
	}

	std::unordered_map<UUID, Model>::iterator RenderTask::endSceneModels() {
		return m_pScene->m_modelComponents.end();
	}

	bool RenderTask::isSameScene(Actor actor) {
		return m_pScene == actor.getScene();
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

	uint32_t RenderTask::getMeshInstanceIndex(UUID actor, UUID mesh) {
		if (m_pScene->m_meshInstanceIndices.count(mesh + actor)) {
			return m_pScene->m_meshInstanceIndices.at(mesh + actor);
		}
		return 0;
	}

	std::unordered_map<UUID, std::unique_ptr<Texture>>* RenderTask::getTextureDataMap() {
		return &Base::getBase()->m_assetHandler->m_textureMap;
	}

	uint32_t RenderTask::getTextureIndex(UUID texture) {
		return Base::getBase()->m_textureIndices.at(texture);
	}

	vk::Sampler* RenderTask::getTextureSampler() {
		return &Base::getBase()->m_textureSampler;
	}

}