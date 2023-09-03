#pragma once

#include "Zap.h"

namespace Zap {
	class Renderer
	{
	public:
		Renderer(Window& window);
		~Renderer();
	
		void init();

		void setViewport(uint32_t width, uint32_t height, uint32_t x, uint32_t y);

	private:
		Window m_window;
		VkViewport m_viewport;
		VkRect2D m_scissor;

		vk::Surface m_surface = vk::Surface();
		vk::Swapchain m_swapchain = vk::Swapchain();
		vk::DescriptorPool m_descriptorPool = vk::DescriptorPool();
		vk::RenderPass m_renderPass = vk::RenderPass();
		vk::Shader m_vertexShader = vk::Shader();
		vk::Shader m_fragmentShader = vk::Shader();
		vk::Pipeline m_pipeline = vk::Pipeline();

		//Buffers
		struct UniformBufferObject {// definition of the uniform buffer layout
			glm::vec3 color;
		};

		UniformBufferObject m_ubo{// the host uniformbuffer
			m_ubo.color = {0, 1, 0}
		};
		vk::Buffer m_uniformBuffer = vk::Buffer();// the vulkan uniforbuffer
	};
}

