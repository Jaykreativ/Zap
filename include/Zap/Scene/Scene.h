#pragma once

#include "Zap/Zap.h"
#include "Zap/UUID.h"
#include "Zap/Events.h"
#include "Zap/Scene/Actor.h"
#include "Zap/Scene/Components.h"

#include "glm.hpp"

#include <vector>

namespace Zap {
	class Actor; // forward declaration
	class LineVertex;
	struct SceneDesc {
		glm::vec3 gravity = {0, -9.81, 0};
	};

	class Scene
	{
	public:
		Scene(std::string name = "", UUID handle = UUID());
		~Scene();

		bool operator ==(const Scene& act) { return m_handle == act.m_handle; }

		std::string name() const;

		void rename(std::string name);

		UUID getHandle();

		bool isValid();

		void init(SceneDesc desc = {});

		void destroy();

		void attachActor(Actor& actor);

		struct RaycastOutput {
			Actor actor;
			float distance;
			glm::vec3 normal;
			glm::vec3 position;
		};
		bool raycast(glm::vec3 origin, glm::vec3 unitDir, uint32_t maxDistance, RaycastOutput* out, physx::PxQueryFilterCallback* filterCallback = nullptr); // cleanup query filter

		void simulate(float elapsedTime);

		void update();

		std::vector<Actor> scanActors();

		std::set<std::filesystem::path> getAssetPaths() const;

		const physx::PxRenderBuffer* getPxRenderBuffer();

		/*
		* Writes all Lines from the PhysX RenderBuffer to the back of the given vector
		*/
		bool getPxDebugVertices(std::vector<LineVertex>& debugVertices);

		SceneEventHandler& getEventHandler();

#ifndef ZP_ALL_PUBLIC
	private:
#endif
		UUID m_handle;
		std::string m_name;

		physx::PxScene* m_pxScene = nullptr;

		//TODO add parent/child system to actors
#ifdef ZP_ENTITY_COMPONENT_SYSTEM_ACCESS
	public:
#endif
		std::unordered_map<UUID, Name>            m_nameComponents;
		std::unordered_map<UUID, Transform>       m_transformComponents;
		std::unordered_map<UUID, Camera>          m_cameraComponents;// TODO rework access system
		std::unordered_map<UUID, Light>           m_lightComponents;
		std::unordered_map<UUID, Model>           m_modelComponents;
		std::unordered_map<UUID, RigidDynamic>    m_rigidDynamicComponents;
		std::unordered_map<UUID, RigidStatic>     m_rigidStaticComponents;
#ifdef ZP_ENTITY_COMPONENT_SYSTEM_ACCESS
#ifndef ZP_ALL_PUBLIC
	private:
#endif
#endif

		SceneEventHandler m_eventHandler;

		struct LightData {
			alignas(16) glm::vec3 pos;
			alignas(16) glm::vec3 color;
			alignas(4) float strength;
			alignas(4) float radius;
		};

		struct PerMeshInstanceData {
			alignas(16) glm::mat4 transform;
			alignas(16) glm::mat4 normalTransform;
			alignas(16) MaterialGpuData material;
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
		friend class RenderTask;
		friend class PBRenderer;
		friend class RaytracingRenderer;
		friend class PathTracer;
	};
}

