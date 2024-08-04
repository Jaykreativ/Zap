#include "Zap/Rendering/RenderTaskTemplate.h"

#include "Zap/Rendering/Renderer.h"
#include "Zap/Scene/Scene.h"
#include "Zap/Scene/Actor.h"
#include "Zap/Scene/Model.h"
#include "Zap/Scene/Mesh.h"

namespace Zap {
	RenderTaskTemplate::RenderTaskTemplate() {}

	RenderTaskTemplate::RenderTaskTemplate(Scene* pScene)
		: m_pScene(pScene)
	{}

	RenderTaskTemplate::~RenderTaskTemplate(){}

	void RenderTaskTemplate::initTargetDependencies() { m_pRenderer->initRenderTaskTargetDependencies(this); }

	void RenderTaskTemplate::resizeTargetDependencies() { m_pRenderer->resizeRenderTaskTargetDependencies(this); }

	vk::Registery* RenderTaskTemplate::getRegistery() {
		return &Base::getBase()->m_registery;
	}

	vk::Buffer* RenderTaskTemplate::getScenePerMeshInstanceBuffer() {
		if (m_pScene) {
			return &m_pScene->m_perMeshInstanceBuffer;
		}
		return nullptr;
	}

	vk::Buffer* RenderTaskTemplate::getSceneLightBuffer() {
		if (m_pScene) {
			return &m_pScene->m_lightBuffer;
		}
		return nullptr;
	}

	Model* RenderTaskTemplate::getActorModel(Actor actor) {
		return &m_pScene->m_modelComponents.at(actor);
	}

	uint32_t RenderTaskTemplate::getMeshInstanceIndex(UUID actor, Mesh mesh) {
		if (m_pScene->m_meshInstanceIndices.count(mesh.getHandle() + (UUID)actor)) {
			return m_pScene->m_meshInstanceIndices.at(mesh.getHandle() + (UUID)actor);
		}
		return 0;
	}
	uint32_t RenderTaskTemplate::getMeshInstanceIndex(Actor actor, Mesh mesh) {
		return getMeshInstanceIndex((UUID)actor, mesh);
	}
}