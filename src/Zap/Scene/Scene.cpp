#include "Zap/Scene/Scene.h"
#include "Zap/Scene/PhysicsComponent.h"

namespace Zap {
	Scene::Scene() {}

	Scene::Scene(UUID handle)
		: m_dataHandle(handle)
	{}

	Scene::~Scene() {

	}

	void Scene::init() {
		auto base = Base::getBase();
		SceneData* data = &base->m_scenes.at(m_dataHandle);

		physx::PxSceneDesc sceneDesc(base->m_pxPhysics->getTolerancesScale());
		sceneDesc.gravity = physx::PxVec3(0.0f, -9.81f, 0.0f);
		sceneDesc.flags |= physx::PxSceneFlag::eENABLE_ACTIVE_ACTORS;
		sceneDesc.cpuDispatcher = physx::PxDefaultCpuDispatcherCreate(1);
		sceneDesc.filterShader = physx::PxDefaultSimulationFilterShader;
		//sceneDesc.simulationEventCallback = &simulationCallbacks;
		data->m_pxScene = base->m_pxPhysics->createScene(sceneDesc);
		if (!data->m_pxScene) {
			std::cerr << "ERROR: createScene failed\n";
			throw std::runtime_error("ERROR: createScene failed");
		}

		physx::PxPvdSceneClient* pvdClient = data->m_pxScene->getScenePvdClient();
		if (pvdClient)
		{
			pvdClient->setScenePvdFlag(physx::PxPvdSceneFlag::eTRANSMIT_CONSTRAINTS, true);
			pvdClient->setScenePvdFlag(physx::PxPvdSceneFlag::eTRANSMIT_CONTACTS, true);
			pvdClient->setScenePvdFlag(physx::PxPvdSceneFlag::eTRANSMIT_SCENEQUERIES, true);
		}
	}

	bool Scene::raycast(glm::vec3 origin, glm::vec3 unitDir, uint32_t maxDistance, RaycastOutput* out, physx::PxQueryFilterCallback* filterCallback) {
		auto base = Base::getBase();
		SceneData* data = &base->m_scenes.at(m_dataHandle);

		physx::PxRaycastBuffer hit;

		auto filterData = physx::PxQueryFilterData();
		filterData.flags |= physx::PxQueryFlag::ePREFILTER;
		

		if (!data->m_pxScene->raycast(
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
		SceneData* data = &base->m_scenes.at(m_dataHandle);

		data->m_pxScene->simulate(elapsedTime);
		data->m_pxScene->fetchResults(true);
		uint32_t numActors = 0;
		auto actors = data->m_pxScene->getActiveActors(numActors);
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