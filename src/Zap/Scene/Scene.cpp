#include "Zap/Scene/Scene.h"
#include "Zap/Scene/PhysicsComponent.h"

namespace Zap {
	Scene::Scene() {

	}

	Scene::~Scene() {

	}

	void Scene::simulate(float elapsedTime) {
		auto base = Base::getBase();
		base->m_pxScene->simulate(elapsedTime);
		base->m_pxScene->fetchResults(true);
		uint32_t numActors = 0;
		auto actors = base->m_pxScene->getActiveActors(numActors);
		for (uint32_t i = 0; i < numActors; i++) {
			auto pxActor = actors[i];
			switch (pxActor->getType()) {
			case physx::PxActorType::eRIGID_DYNAMIC: {
				auto cmp = &RigidDynamicComponent::all[(uint32_t)pxActor->userData];
				auto pxMat = physx::PxMat44(((physx::PxRigidDynamic*)pxActor)->getGlobalPose());
				glm::mat4 mat = *((glm::mat4*)&pxMat);
				glm::mat4& transform = cmp->m_pActor->getTransform();
				mat[0] = mat[0] * glm::length(transform[0]);
				mat[1] = mat[1] * glm::length(transform[1]);
				mat[2] = mat[2] * glm::length(transform[2]);
				cmp->m_pActor->setTransform(mat);
			}
			}
		}
	}
}