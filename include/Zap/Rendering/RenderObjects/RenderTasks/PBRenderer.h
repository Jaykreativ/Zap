#pragma once

#include "Zap/Zap.h"
#include "Zap/Scene/Actor.h"
#include "Zap/Scene/Material.h"
#include "Zap/Rendering/RenderObjects/Framebuffer.h"
#include "Zap/Rendering/RenderObjects/RenderTargets.h"
#include "Zap/Rendering/RenderObjects/RenderTask.h"
#include "Zap/Rendering/RenderObjects/DescriptorSet.h"

#include "glm.hpp"

namespace Zap {
	class Scene;

	class PBRenderer : public RenderTask,
		public EventListener<AssetHandlerEvent::TextureLoad>,
		public EventListener<SceneEvent::UpdateMeshInstanceBuffer>,
		public EventListener<SceneEvent::UpdateLightBuffer>
	{
	public:
		glm::vec4 clearColor        = { 0.0f, 0.0f, 0.0f, 1.0f };
		glm::vec2 clearDepthStencil = { 1.0f, 0.0f };

		PBRenderer(Renderer* pRenderer, RenderTargetHandle<> target, Scene* pScene);
		PBRenderer(const PBRenderer& pbrenderer);
		~PBRenderer();

		void updateCamera(Actor camera);

		void changeScene(Scene* pScene); // TODO implement change scene in render

		void setViewport(uint32_t width, uint32_t height, uint32_t x, uint32_t y);

		void getViewport(uint32_t& width, uint32_t& height, uint32_t& x, uint32_t& y);

	private:
		Scene* m_pScene = nullptr;

		VkViewport m_viewport;
		VkRect2D m_scissor;

		vk::RenderPass m_renderPass = vk::RenderPass();

		RenderTargetHandle<> m_target;
		RenderTargetHandle<RenderTargetImage> m_depthTarget;

		FramebufferHandle m_framebuffer;

		DescriptorSetHandle<GenericDescriptorSet> m_descriptorSet;
		DescriptorSetHandle<GenericDescriptorSet> m_textureSet;

		vk::Shader m_vertexShader = vk::Shader();
		vk::Shader m_fragmentShader = vk::Shader();
		vk::Pipeline m_pipeline = vk::Pipeline();

		//Semaphores
		VkSemaphore m_semaphoreRenderComplete;

		//Buffers
		struct UniformBufferObject {// definition of the uniform buffer layout
			glm::mat4 view;
			glm::mat4 perspective;
			alignas(16) glm::vec3 camPos;
			alignas(16) glm::vec3 color;
			alignas(4) uint32_t lightCount;
		};

		UniformBufferObject m_ubo{};// the host uniform buffer
		vk::Buffer m_uniformBuffer = vk::Buffer();// the vulkan uniform buffer;

		bool m_areTexturesOutdated = false;

		uint32_t m_loadedTextureCount = 0;

		void init(const LayoutTransitionHelper& layoutTransitionHelper) override;

		void destroy() override;

		void recordCommands(const vk::CommandBuffer* cmd) override;

		void beforeRender() override;

		void addDescriptorPoolSizes(DescriptorPoolSizeList& poolSizes) override;

		TaskLayoutTransitions getLayoutTransitions() override;

		void updateTextureDescriptor();

		void callback(const AssetHandlerEvent::TextureLoad& event) override;

		void callback(const SceneEvent::UpdateLightBuffer& event) override;

		void callback(const SceneEvent::UpdateMeshInstanceBuffer& event) override;
	};
}