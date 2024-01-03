#include "Zap/Scene/Scene.h"
#include "Zap/Scene/Actor.h"
#include "Zap/Scene/PhysicsComponent.h"

namespace Zap {
	Scene::Scene() {}

	Scene::~Scene() {

	}

	void Scene::init() {
		auto base = Base::getBase();

		physx::PxSceneDesc sceneDesc(base->m_pxPhysics->getTolerancesScale());
		sceneDesc.gravity = physx::PxVec3(0.0f, -9.81f, 0.0f);
		sceneDesc.flags |= physx::PxSceneFlag::eENABLE_ACTIVE_ACTORS;
		sceneDesc.cpuDispatcher = physx::PxDefaultCpuDispatcherCreate(1);
		sceneDesc.filterShader = physx::PxDefaultSimulationFilterShader;
		//sceneDesc.simulationEventCallback = &simulationCallbacks;
		m_pxScene = base->m_pxPhysics->createScene(sceneDesc);
		if (!m_pxScene) {
			std::cerr << "ERROR: createScene failed\n";
			throw std::runtime_error("ERROR: createScene failed");
		}

		physx::PxPvdSceneClient* pvdClient = m_pxScene->getScenePvdClient();
		if (pvdClient)
		{
			pvdClient->setScenePvdFlag(physx::PxPvdSceneFlag::eTRANSMIT_CONSTRAINTS, true);
			pvdClient->setScenePvdFlag(physx::PxPvdSceneFlag::eTRANSMIT_CONTACTS, true);
			pvdClient->setScenePvdFlag(physx::PxPvdSceneFlag::eTRANSMIT_SCENEQUERIES, true);
		}
	}

	void Scene::destroy() {
		for (auto const& x : m_rigidDynamicComponents) x.second.pxActor->release();
		for (auto const& x : m_rigidStaticComponents) x.second.pxActor->release();
	}

	void Scene::attachActor(Actor& actor) {
		actor.m_pScene = this;
	}

	bool Scene::raycast(glm::vec3 origin, glm::vec3 unitDir, uint32_t maxDistance, RaycastOutput* out, physx::PxQueryFilterCallback* filterCallback) {
		auto base = Base::getBase();

		physx::PxRaycastBuffer hit;

		auto filterData = physx::PxQueryFilterData();
		filterData.flags |= physx::PxQueryFlag::ePREFILTER;
		

		if (!m_pxScene->raycast(
			*((physx::PxVec3*)&origin),
			*((physx::PxVec3*)&unitDir),
			maxDistance,
			hit,
			physx::PxHitFlag::eDEFAULT,
			filterData,
			filterCallback
		)) return false;

		out->actor = Actor(UUID((uint64_t)hit.block.actor->userData), this);
		out->distance = hit.block.distance;
		out->normal = *((glm::vec3*)&hit.block.normal);
		out->position = *((glm::vec3*)&hit.block.position);

		return true;
	}

	void Scene::simulate(float elapsedTime) {
		if (elapsedTime <= 0) return;
		auto base = Base::getBase();

		m_pxScene->simulate(elapsedTime);
		m_pxScene->fetchResults(true);
		uint32_t numActors = 0;
		auto actors = m_pxScene->getActiveActors(numActors);
		for (uint32_t i = 0; i < numActors; i++) {
			auto pxActor = actors[i];
			switch (pxActor->getType()) {
			case physx::PxActorType::eRIGID_DYNAMIC: {
				RigidDynamicComponent* cmp = &m_rigidDynamicComponents.at((uint64_t)pxActor->userData);
				glm::mat4* transform = &m_transformComponents.at((uint64_t)pxActor->userData).transform;
				physx::PxMat44 pxMat = physx::PxMat44(((physx::PxRigidDynamic*)pxActor)->getGlobalPose());
				glm::mat4 mat = *((glm::mat4*)&pxMat);
				mat[0] = mat[0] * glm::length((*transform)[0]);
				mat[1] = mat[1] * glm::length((*transform)[1]);
				mat[2] = mat[2] * glm::length((*transform)[2]);
				*transform = { mat };
			}
			}
		}
	}
}