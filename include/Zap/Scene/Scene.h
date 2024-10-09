#pragma once

#include "Zap/Zap.h"
#include "Zap/UUID.h"
#include "Zap/Event.h"
#include "Zap/EventHandler.h"
#include "Zap/Scene/Actor.h"
#include "Zap/Scene/Camera.h"
#include "Zap/Scene/Light.h"
#include "Zap/Scene/Model.h"
#include "Zap/Scene/PhysicsComponent.h"
#include "Zap/Scene/Transform.h"

#include "glm.hpp"

#include <vector>

namespace Zap {
	class Actor; // forward declaration
	class DebugRenderVertex;

	class SceneUpdateEvent : public Event {
	public:
		SceneUpdateEvent(Scene* pScene)
			: pScene(pScene)
		{};
		~SceneUpdateEvent() = default;

		Scene* pScene = nullptr;
	};

	class AddActorEvent : public Event {
	public:
		AddActorEvent(Scene* pScene, Actor actor)
			: pScene(pScene), actor(actor)
		{};
		~AddActorEvent() = default;

		Scene* pScene;
		Actor actor;
	};

	class RemoveActorEvent : public Event {
	public:
		RemoveActorEvent(Scene* pScene, Actor actor)
			: pScene(pScene), actor(actor)
		{};
		~RemoveActorEvent() = default;

		Scene* pScene;
		Actor actor;
	};
	
	class AddLightEvent : public AddActorEvent {
	public:
		AddLightEvent(Scene* pScene, Actor actor, uint32_t lightCount)
			: AddActorEvent(pScene, actor), lightCount(lightCount)
		{};
		~AddLightEvent() = default;

		uint32_t lightCount = 0;
	};

	class RemoveLightEvent : public RemoveActorEvent {
	public:
		RemoveLightEvent(Scene* pScene, Actor actor, uint32_t lightCount)
			: RemoveActorEvent(pScene, actor), lightCount(lightCount)
		{};
		~RemoveLightEvent() = default;

		uint32_t lightCount = 0;
	};

	class AddModelEvent : public AddActorEvent {
	public:
		AddModelEvent(Scene* pScene, Actor actor, uint32_t modelCount)
			: AddActorEvent(pScene, actor), modelCount(modelCount)
		{};
		~AddModelEvent() = default;

		uint32_t modelCount = 0;
	};

	class RemoveModelEvent : public RemoveActorEvent {
	public:
		RemoveModelEvent(Scene* pScene, Actor actor, uint32_t modelCount)
			: RemoveActorEvent(pScene, actor), modelCount(modelCount)
		{};
		~RemoveModelEvent() = default;

		uint32_t modelCount = 0;
	};

	class Scene
	{
	public:
		Scene();
		Scene(UUID handle);
		~Scene();

		struct RaycastOutput {
			Actor actor;
			float distance;
			glm::vec3 normal;
			glm::vec3 position;
		};

		void init();

		void destroy();

		void attachActor(Actor& actor);

		bool raycast(glm::vec3 origin, glm::vec3 unitDir, uint32_t maxDistance, RaycastOutput* out, physx::PxQueryFilterCallback* filterCallback = nullptr); // cleanup query filter

		void simulate(float elapsedTime);

		void update();

		const physx::PxRenderBuffer* getPxRenderBuffer();

		/*
		* Writes all Lines from the PhysX RenderBuffer to the back of the given vector
		*/
		bool getPxDebugVertices(std::vector<DebugRenderVertex>& debugVertices);

		EventHandler<SceneUpdateEvent>* getSceneUpdateEventHandler();

		EventHandler<AddActorEvent>* getAddActorEventHandler();
		
		EventHandler<AddLightEvent>* getAddLightEventHandler();

		EventHandler<AddModelEvent>* getAddModelEventHandler();

		EventHandler<RemoveActorEvent>* getRemoveActorEventHandler();

		EventHandler<RemoveLightEvent>* getRemoveLightEventHandler();

		EventHandler<RemoveModelEvent>* getRemoveModelEventHandler();

#ifndef ZP_ALL_PUBLIC
	private:
#endif
		UUID m_handle;

		physx::PxScene* m_pxScene = nullptr;

		//TODO add parent/child system to actors
#ifdef ZP_ENTITY_COMPONENT_SYSTEM_ACCESS
	public:
#endif
		std::unordered_map<UUID, Camera>          m_cameraComponents;// TODO rework access system
		std::unordered_map<UUID, Light>           m_lightComponents;
		std::unordered_map<UUID, Model>           m_modelComponents;
		std::unordered_map<UUID, RigidDynamic>    m_rigidDynamicComponents;
		std::unordered_map<UUID, RigidStatic>     m_rigidStaticComponents;
		std::unordered_map<UUID, Transform>       m_transformComponents;
#ifdef ZP_ENTITY_COMPONENT_SYSTEM_ACCESS
#ifndef ZP_ALL_PUBLIC
	private:
#endif
#endif

		EventHandler<SceneUpdateEvent> m_sceneUpdateEventHandler;
		EventHandler<AddActorEvent> m_addActorEventHandler;
		EventHandler<AddLightEvent> m_addLightEventHandler;
		EventHandler<AddModelEvent> m_addModelEventHandler;
		EventHandler<RemoveActorEvent> m_removeActorEventHandler;
		EventHandler<RemoveLightEvent> m_removeLightEventHandler;
		EventHandler<RemoveModelEvent> m_removeModelEventHandler;

		struct LightData {
			alignas(16) glm::vec3 pos;
			alignas(16) glm::vec3 color;
			alignas(4) float strength;
			alignas(4) float radius;
		};

		struct PerMeshInstanceData {
			alignas(16) glm::mat4 transform;
			alignas(16) glm::mat4 normalTransform;
			alignas(16) Material material;
			alignas(8) VkDeviceAddress vertexBufferAddress;
			alignas(8) VkDeviceAddress indexBufferAddress;
		};

		uint32_t m_meshInstanceCount = 0;

		vk::Buffer m_perMeshInstanceBuffer;
		vk::Buffer m_lightBuffer;
		
		std::unordered_map<UUID, uint32_t> m_meshInstanceIndices;

		friend class Base;
		friend class Serializer;
		friend class Actor;
		friend class RigidDynamic;
		friend class RigidStatic;
		friend class RenderTaskTemplate;
		friend class PBRenderer;
		friend class RaytracingRenderer;
		friend class PathTracer;
	};
}

