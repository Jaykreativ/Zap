#pragma once

#include "Zap/Zap.h"
#include "Zap/Rendering/RenderObject.h"
#include "Zap/AssetHandling/Asset.h"
#include "Zap/AssetHandling/AssetTypes/Texture.h"

namespace Zap {
	class Model;
	class Mesh;
	class Texture;
	class Actor;
	class LayoutTransitionHelper;
	class TaskLayoutTransitions;
	class DescriptorPoolSizeList;

	class RenderTask : public RenderObject {
	public:
		RenderTask(Renderer* pRenderer);
		// Can be used by scene dependant tasks to gain access to private storage buffers
		RenderTask(Renderer* pRenderer, Scene* pScene);

		virtual ~RenderTask();

		void disable() { m_isEnabled = false; }

		void enable() { m_isEnabled = true; }

	protected:
		// Only works when a scene ptr was supplied in the constructor
		std::unordered_map<UUID, Model>::iterator beginSceneModels();

		// Only works when a scene ptr was supplied in the constructor
		std::unordered_map<UUID, Model>::iterator endSceneModels();

		// Only works when a scene ptr was supplied in the constructor
		// checks whether the currently rendered scene contains the given actor
		bool isSameScene(Actor actor);

		// returns a ptr to the engines global vulkan registery object
		vk::Registery* getRegistery();

		// Only works when a scene ptr was supplied in the constructor
		// Returns nullptr when failed
		vk::Buffer* getScenePerMeshInstanceBuffer();

		// Only works when a scene ptr was supplied in the constructor
		// Returns nullptr when failed
		vk::Buffer* getSceneLightBuffer();

		// Only works when a scene ptr was supplied in the constructor
		// Returns nullptr when failed
		Model* getActorModel(Actor actor);

		// Only works when a scene ptr was supplied in the constructor
		// Returns 0 when failing
		uint32_t getMeshInstanceIndex(UUID actor, UUID mesh);

		std::unordered_map<UUID, std::unique_ptr<Texture>>* getTextureDataMap();

		uint32_t getTextureIndex(UUID texture);

		vk::Sampler* getTextureSampler();

	private:
		bool m_isEnabled = true;
		Scene* m_pScene = nullptr;

		virtual void init(const LayoutTransitionHelper& layoutTransitionHelper) = 0;

		virtual void destroy() = 0;

		virtual void recordCommands(const vk::CommandBuffer* cmd) = 0;

		virtual void beforeRender() {};

		virtual void afterRender() {};

		virtual void addDescriptorPoolSizes(DescriptorPoolSizeList& poolSizes) = 0;

		virtual TaskLayoutTransitions getLayoutTransitions() = 0;

		friend class Renderer;
	};

	// handle to a RenderTask stored in a Renderer
	// invalid if the Renderer was destroyed
	template<class T = RenderTask>
	class RenderTaskHandle {
		friend class Renderer;
		template<class U>
		friend class RenderTaskHandle;
	public:
		RenderTaskHandle() = default;
		RenderTaskHandle(const RenderTaskHandle<T>& other)
			: m_handle(other.m_handle), m_renderer(other.m_renderer)
		{}
		~RenderTaskHandle() = default;

		operator RenderTaskHandle<RenderTask>() {
			return RenderTaskHandle<RenderTask>(m_handle, m_renderer);
		}

		T* get() {
			return reinterpret_cast<T*>(m_renderer->getRenderTask(m_handle));
		}
		const T* get() const {
			return reinterpret_cast<T*>(m_renderer->getRenderTask(m_handle));
		}

		T* operator->() {
			return get();
		}

		operator bool() const {
			return m_renderer != nullptr && get() != nullptr;
		}

		void reset() {
			m_handle = 0;
			m_renderer = nullptr;
		}

	private:
		UUID m_handle = 0;
		Renderer* m_renderer = nullptr;

		RenderTaskHandle(UUID handle, Renderer* renderer)
			: m_handle(handle), m_renderer(renderer)
		{}
	};
}