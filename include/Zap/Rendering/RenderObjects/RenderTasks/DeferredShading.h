#pragma once

#include "Zap/Zap.h"
#include "Zap/Events.h"
#include "Zap/Rendering/RenderObjects/RenderTask.h"

#include "glm.hpp"

namespace Zap {
	class GeometryPass : public RenderTask,
		public EventListener<AssetHandlerEvent::TextureLoad>
	{
	public:
		glm::vec4 clearColor = { 0.0f, 0.0f, 0.0f, 1.0f };
		glm::vec2 clearDepthStencil = { 1.0f, 0.0f };

		GeometryPass(Renderer* pRenderer, Scene* pScene);
		~GeometryPass();

		void updateCamera(Actor camera);

		void changeScene(Scene* pScene); // TODO implement change scene in render

		void setViewport(uint32_t width, uint32_t height, uint32_t x, uint32_t y);

		void getViewport(uint32_t& width, uint32_t& height, uint32_t& x, uint32_t& y);

	private:
		Scene* m_pScene = nullptr;

		VkViewport m_viewport;
		VkRect2D m_scissor;

		vk::RenderPass m_renderPass = vk::RenderPass();

		vk::Image m_depthImage;

		std::vector<vk::Framebuffer> m_framebuffers;

		vk::DescriptorPool m_descriptorPool = vk::DescriptorPool();
		vk::DescriptorSet m_descriptorSet = vk::DescriptorSet();
		vk::DescriptorSet m_textureSet = vk::DescriptorSet();

		vk::Shader m_vertexShader = vk::Shader();
		vk::Shader m_fragmentShader = vk::Shader();
		vk::Pipeline m_pipeline = vk::Pipeline();

		//Semaphores
		VkSemaphore m_semaphoreRenderComplete;

		//Buffers
		struct UniformBufferObject {// definition of the uniform buffer layout
			glm::mat4 view;
			glm::mat4 perspective;
		};

		UniformBufferObject m_ubo{};// the host uniform buffer
		vk::Buffer m_uniformBuffer = vk::Buffer();// the vulkan uniform buffer;

		bool m_areTexturesOutdated = false;

		uint32_t m_loadedTextureCount = 0;

		void init(uint32_t width, uint32_t height, uint32_t imageCount);

		void initTargetDependencies(uint32_t width, uint32_t height, uint32_t imageCount, vk::Image* pTarget, uint32_t imageIndex);

		void resize(uint32_t width, uint32_t height, uint32_t imageCount);

		void resizeTargetDependencies(uint32_t width, uint32_t height, uint32_t imageCount, vk::Image* pTarget, uint32_t imageIndex);

		void destroy();

		void beforeRender(vk::Image* pTarget, uint32_t imageIndex);

		void afterRender(vk::Image* pTarget, uint32_t imageIndex);

		void recordCommands(const vk::CommandBuffer* cmd, vk::Image* pTarget, uint32_t imageIndex);

		void updateTextureDescriptor();

		void callback(const AssetHandlerEvent::TextureLoad& event) override;
    };
}