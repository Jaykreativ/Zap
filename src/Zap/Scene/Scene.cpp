#include "Zap/Scene/Scene.h"
#include "Zap/Scene/PhysicsComponent.h"

namespace Zap {
	Scene::Scene() {

	}

	Scene::~Scene() {

	}

	bool Scene::raycast(glm::vec3 origin, glm::vec3 unitDir, uint32_t maxDistance, RaycastOutput* out, physx::PxQueryFilterCallback* filterCallback) {
		auto base = Base::getBase();

		physx::PxRaycastBuffer hit;

		auto filterData = physx::PxQueryFilterData();
		filterData.flags |= physx::PxQueryFlag::ePREFILTER;
		

		if (!base->m_pxScene->raycast(
			*((physx::PxVec3*)&origin),
			*((physx::PxVec3*)&unitDir),
			maxDistance,
			hit,
			physx::PxHitFlag::eDEFAULT,
			filterData,
			filterCallback
		)) return false;

		switch (hit.block.actor->getType()) {
		case physx::PxActorType::eRIGID_DYNAMIC: {
			out->pActor = RigidDynamicComponent::all[(uint64_t)hit.block.actor->userData].m_pActor;
		}
		case physx::PxActorType::eRIGID_STATIC: {
			out->pActor = RigidStaticComponent::all[(uint64_t)hit.block.actor->userData].m_pActor;
		}
		}
		
		out->distance = hit.block.distance;
		out->normal = *((glm::vec3*)&hit.block.normal);
		out->position = *((glm::vec3*)&hit.block.position);

		return true;
	}

	void Scene::simulate(float elapsedTime) {
		if (elapsedTime <= 0) return;
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