#include "Zap/Scene/Scene.h"
#include "Zap/Scene/Actor.h"
#include "Zap/Scene/Mesh.h"
#include "Zap/Scene/Material.h"
#include "Zap/Physics/PhysicsComponent.h"
#include "Zap/Rendering/DebugRenderTask.h"

namespace Zap {
	Scene::Scene()
		: m_handle()
	{}
	Scene::Scene(UUID handle)
		: m_handle(handle)
	{}

	Scene::~Scene() {}

	UUID Scene::getHandle() {
		return m_handle;
	}

	bool Scene::isValid() {
		return (bool)m_handle;
	}

	void Scene::init(SceneDesc desc) {
		auto base = Base::getBase();

		physx::PxSceneDesc sceneDesc(base->m_pxPhysics->getTolerancesScale());
		sceneDesc.gravity = Zap::PxUtils::glmVec3toVec3(desc.gravity);
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

		m_pxScene->setVisualizationParameter(physx::PxVisualizationParameter::eSCALE, 1.0f);
		m_pxScene->setVisualizationParameter(physx::PxVisualizationParameter::eCOLLISION_SHAPES, 1.0f);

		m_perMeshInstanceBuffer = vk::Buffer(m_meshInstanceCount*sizeof(PerMeshInstanceData), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
		m_perMeshInstanceBuffer.init(); m_perMeshInstanceBuffer.allocate(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

		m_lightBuffer = vk::Buffer(m_lightComponents.size()*sizeof(LightData), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
		m_lightBuffer.init(); m_lightBuffer.allocate(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

		m_meshInstanceIndices = {};
	}

	void Scene::update() {
		auto* base = Base::getBase();

		if (m_lightComponents.size() * sizeof(LightData) != m_lightBuffer.getSize()) {
			m_lightBuffer.resize(m_lightComponents.size() * sizeof(LightData));
			m_lightBuffer.update();
		}

		void* rawData;
		if (m_lightBuffer.getSize() > 0) {
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
		}

		// resize buffer and fill in one time data
		if (m_meshInstanceCount * sizeof(PerMeshInstanceData) != m_perMeshInstanceBuffer.getSize()) {
			m_meshInstanceIndices.clear();

			m_perMeshInstanceBuffer.resize(std::max<size_t>(m_meshInstanceCount, 1) * sizeof(PerMeshInstanceData));
			m_perMeshInstanceBuffer.map(&rawData);
			{
				PerMeshInstanceData* perMeshInstance = (PerMeshInstanceData*)(rawData);
				uint32_t i = 0;
				for (auto const& modelPair : m_modelComponents) {
					for (Mesh mesh : modelPair.second.meshes) {
						perMeshInstance[i].vertexBufferAddress = mesh.getVertexBuffer()->getVkDeviceAddress();
						perMeshInstance[i].indexBufferAddress = mesh.getIndexBuffer()->getVkDeviceAddress();

						m_meshInstanceIndices[mesh.getHandle()+modelPair.first] = i;
						i++;
					}
				}
			}
			m_perMeshInstanceBuffer.unmap();
		}

		if (m_perMeshInstanceBuffer.getSize() > 0) {
			m_perMeshInstanceBuffer.map(&rawData);
			{
				PerMeshInstanceData* perMeshInstance = (PerMeshInstanceData*)(rawData);
				uint32_t i = 0;
				for (auto& modelPair : m_modelComponents) {
					uint32_t j = 0;
					for (Mesh mesh : modelPair.second.meshes) {
						auto* base = Base::getBase();
						perMeshInstance[i].transform = m_transformComponents.at(modelPair.first).transform * *mesh.getTransform();
						perMeshInstance[i].normalTransform = glm::transpose(glm::inverse(perMeshInstance[i].transform));
						auto& material = *base->m_assetHandler.getMaterialDataPtr(modelPair.second.materials[j].getHandle());
						MaterialGpuData gpuMaterial = {
							material.albedoColor,
							0xFFFFFFFF,
							material.metallic,
							0xFFFFFFFF,
							material.roughness,
							0xFFFFFFFF,
							material.emissive,
							0xFFFFFFFF,
						};
						if (base->m_textureIndices.count(material.albedoMap.getHandle()))
							gpuMaterial.albedoMap = base->m_textureIndices.at(material.albedoMap.getHandle());
						if (base->m_textureIndices.count(material.metallicMap.getHandle()))
							gpuMaterial.metallicMap = base->m_textureIndices.at(material.metallicMap.getHandle());
						if (base->m_textureIndices.count(material.roughnessMap.getHandle()))
							gpuMaterial.roughnessMap = base->m_textureIndices.at(material.roughnessMap.getHandle());
						if (base->m_textureIndices.count(material.emissiveMap.getHandle()))
							gpuMaterial.emissiveMap = base->m_textureIndices.at(material.emissiveMap.getHandle());
						perMeshInstance[i].material = gpuMaterial;
						j++; i++;

					}
				}
			}
			m_perMeshInstanceBuffer.unmap();
		}

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
				glm::mat4 mat = PxUtils::transformToGlmMat4(((physx::PxRigidDynamic*)pxActor)->getGlobalPose());
				mat[0] = mat[0] * glm::length((*transform)[0]);
				mat[1] = mat[1] * glm::length((*transform)[1]);
				mat[2] = mat[2] * glm::length((*transform)[2]);
				*transform = mat;
			}
			}
		}
	}

	const physx::PxRenderBuffer* Scene::getPxRenderBuffer() {
		return &m_pxScene->getRenderBuffer();
	}

	bool Scene::getPxDebugVertices(std::vector<DebugRenderVertex>& debugVertices) {
		if (!m_pxScene)
			return false;

		auto* renderBuffer = getPxRenderBuffer();
		size_t offset = debugVertices.size();
		uint32_t pxSize = renderBuffer->getNbLines();
		auto* lines = renderBuffer->getLines();

		debugVertices.resize(pxSize*2+offset);
		for (uint32_t i = 0; i < pxSize; i++) {
			uint8_t r0 = lines[i].color0;
			uint8_t g0 = lines[i].color0 >> 8;
			uint8_t b0 = lines[i].color0 >> 16;
			uint8_t r1 = lines[i].color1;
			uint8_t g1 = lines[i].color1 >> 8;
			uint8_t b1 = lines[i].color1 >> 16;
			debugVertices[i * 2 + offset]     = Zap::DebugRenderVertex({ lines[i].pos0.x, lines[i].pos0.y, lines[i].pos0.z }, { r0, g0, b0 });
			debugVertices[i * 2 + 1 + offset] = Zap::DebugRenderVertex({ lines[i].pos1.x, lines[i].pos1.y, lines[i].pos1.z }, { r1, g1, b1 });
		}
		return true;
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

	EventHandler<AddModelEvent>* Scene::getAddModelEventHandler() {
		return &m_addModelEventHandler;
	}

	EventHandler<RemoveActorEvent>* Scene::getRemoveActorEventHandler() {
		return &m_removeActorEventHandler;
	}

	EventHandler<RemoveLightEvent>* Scene::getRemoveLightEventHandler() {
		return &m_removeLightEventHandler;
	}

	EventHandler<RemoveModelEvent>* Scene::getRemoveModelEventHandler() {
		return &m_removeModelEventHandler;
	}
}