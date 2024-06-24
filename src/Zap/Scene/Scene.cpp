#include "Zap/Scene/Scene.h"
#include "Zap/Scene/Actor.h"
#include "Zap/Scene/Mesh.h"
#include "Zap/Scene/PhysicsComponent.h"

namespace Zap {
	Scene::Scene() {
		m_handle = UUID();
	}
	Scene::Scene(UUID handle)
		: m_handle(handle)
	{}

	Scene::~Scene() {}

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

		m_perMeshInstanceBuffer = vk::Buffer(std::max<size_t>(m_meshInstanceCount, 1)*sizeof(PerMeshInstanceData), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
		m_perMeshInstanceBuffer.init(); m_perMeshInstanceBuffer.allocate(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

		m_lightBuffer = vk::Buffer(std::max<size_t>(m_lightComponents.size(), 1)*sizeof(LightData), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
		m_lightBuffer.init(); m_lightBuffer.allocate(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
	}

	void Scene::update() {
		if (std::max<size_t>(m_lightComponents.size(), 1) * sizeof(LightData) != m_lightBuffer.getSize()) {
			m_lightBuffer.resize(std::max<size_t>(m_lightComponents.size(), 1) * sizeof(LightData));
			m_lightBuffer.update();
		}

		void* rawData;
		m_lightBuffer.map(&rawData);
		{
			LightData* lightData = (LightData*)(rawData);
			uint32_t i = 0;
			for (auto const& lightPair : m_lightComponents) {
				lightData[i].pos = m_transformComponents.at(lightPair.first).transform[3];
				lightData[i].color = lightPair.second.color;
				lightData[i].strength = lightPair.second.strength;
				lightData[i].radius = lightPair.second.radius;
				i++;
			}
		}
		m_lightBuffer.unmap();

		// resize buffer and fill in one time data
		if (std::max<size_t>(m_meshInstanceCount, 1) * sizeof(PerMeshInstanceData) != m_perMeshInstanceBuffer.getSize()) {
			m_perMeshInstanceBuffer.resize(std::max<size_t>(m_meshInstanceCount, 1) * sizeof(PerMeshInstanceData));
			m_perMeshInstanceBuffer.map(&rawData);
			{
				PerMeshInstanceData* perMeshInstance = (PerMeshInstanceData*)(rawData);
				uint32_t i = 0;
				for (auto const& modelPair : m_modelComponents) {
					for (uint32_t id : modelPair.second.meshes) {
						auto* base = Base::getBase();
						auto mesh = base->m_meshes[id];
						perMeshInstance[i].vertexBufferAddress = mesh.getVertexBuffer()->getVkDeviceAddress();
						perMeshInstance[i].indexBufferAddress = mesh.getIndexBuffer()->getVkDeviceAddress();
						i++;
					}
				}
			}
			m_perMeshInstanceBuffer.unmap();
		}

		m_perMeshInstanceBuffer.map(&rawData);
		{
			PerMeshInstanceData* perMeshInstance = (PerMeshInstanceData*)(rawData);
			uint32_t i = 0;
			for (auto const& modelPair : m_modelComponents) {
				uint32_t j = 0;
				for (uint32_t id : modelPair.second.meshes) {
					auto* base = Base::getBase();
					auto mesh = base->m_meshes[id];
					perMeshInstance[i].transform = m_transformComponents.at(modelPair.first).transform * mesh.m_transform;
					perMeshInstance[i].normalTransform = glm::transpose(glm::inverse(perMeshInstance[i].transform));
					perMeshInstance[i].material = modelPair.second.materials[j];
					j++; i++;
				}
			}
		}
		m_perMeshInstanceBuffer.unmap();

		m_sceneUpdateEventHandler.pushEvent(SceneUpdateEvent(this));
	}

	void Scene::destroy() {
		m_perMeshInstanceBuffer.destroy();
		m_lightBuffer.destroy();
		for (auto const& x : m_rigidDynamicComponents) x.second.pxActor->release();
		for (auto const& x : m_rigidStaticComponents) x.second.pxActor->release();
	}

	void Scene::attachActor(Actor& actor) {
		actor.m_pScene = this;
		m_addActorEventHandler.pushEvent(AddActorEvent(this, actor));
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
				RigidDynamic* cmp = &m_rigidDynamicComponents.at((uint64_t)pxActor->userData);
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

	EventHandler<SceneUpdateEvent>* Scene::getSceneUpdateEventHandler() {
		return &m_sceneUpdateEventHandler;
	}

	EventHandler<AddActorEvent>* Scene::getAddActorEventHandler() {
		return &m_addActorEventHandler;
	}

	EventHandler<AddLightEvent>* Scene::getAddLightEventHandler() {
		return &m_addLightEventHandler;
	}

	EventHandler<RemoveActorEvent>* Scene::getRemoveActorEventHandler() {
		return &m_removeActorEventHandler;
	}

	EventHandler<RemoveLightEvent>* Scene::getRemoveLightEventHandler() {
		return &m_removeLightEventHandler;
	}
}