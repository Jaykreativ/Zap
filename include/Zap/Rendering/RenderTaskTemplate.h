#pragma once

#include "Zap/Zap.h"

namespace Zap {
	class Model;
	class Mesh;
	class Texture;
	class Actor;

	class RenderTaskTemplate {
	public:
		RenderTaskTemplate();
		// Can be used by scene dependant tasks to gain access to private storage buffers
		RenderTaskTemplate(Scene* pScene);

		~RenderTaskTemplate();

		void disable() { m_isEnabled = false; }

		void enable() { m_isEnabled = true; }

	protected:
		// Executes the overwritten initTargetDependencies function
		// Can be called in the init function
		// Should not be overwritten
		void initTargetDependencies();

		// Executes the overwritten resizeTargetDependencies function
		// Can be called in the resize function
		// Should not be overwritten
		void resizeTargetDependencies();

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
		uint32_t getMeshInstanceIndex(UUID actor, Mesh mesh);
		uint32_t getMeshInstanceIndex(Actor actor, Mesh mesh);

		std::unordered_map<UUID, TextureData>* getTextureDataMap();

		uint32_t getTextureIndex(UUID texture);

	private:
		bool m_isEnabled = true;
		Scene* m_pScene = nullptr;
		Renderer* m_pRenderer = nullptr;

		virtual void init(uint32_t width, uint32_t height, uint32_t imageCount) = 0;

		//Will be imageCount times executed
		//Target extent will remain the same
		virtual void initTargetDependencies(uint32_t width, uint32_t height, uint32_t imageCount, vk::Image* pTarget, uint32_t imageIndex) = 0;

		//Will be called when the target gets resized
		virtual void resize(uint32_t width, uint32_t height, uint32_t imageCount) = 0;

		//Will be called when the target gets resized
		//Will be imageCount times executed
		//Target extent will remain the same
		virtual void resizeTargetDependencies(uint32_t width, uint32_t height, uint32_t imageCount, vk::Image* pTarget, uint32_t imageIndex) = 0;

		virtual void destroy() = 0;

		virtual void beforeRender(vk::Image* pTarget, uint32_t imageIndex) = 0;

		virtual void afterRender(vk::Image* pTarget, uint32_t imageIndex) = 0;

		virtual void recordCommands(const vk::CommandBuffer* cmd, vk::Image* pTarget, uint32_t imageIndex) = 0;

		friend class Renderer;
	};
}