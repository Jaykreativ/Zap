#pragma once

#include "glm.hpp"
#include "Zap.h"
#include "Window.h"
#include "Vertex.h"

namespace Zap {
	class Renderer
	{
	public:
		Renderer(Window* window);
		~Renderer();
	
		void init();

		void recordCommandBuffers();

		void render();

		void setViewport(uint32_t width, uint32_t height, uint32_t x, uint32_t y);

		//TODO make actor system
		std::vector<Vertex> vertexArray = {
			Vertex({-0.5f, 0.5f, 0}),
			Vertex({0.5f, 0.5f, 0}),
			Vertex({0, -0.5f, 0})
		};
		
		std::vector<uint32_t> indexArray = {
			0, 1, 2
		};

	private:
		bool m_isInit = false;

		Window* m_window;
		VkViewport m_viewport;
		VkRect2D m_scissor;

		uint32_t m_descriptorSetIndex;

		vk::Shader m_vertexShader = vk::Shader();
		vk::Shader m_fragmentShader = vk::Shader();
		vk::Pipeline m_pipeline = vk::Pipeline();
		
		uint32_t m_commandBufferCount;
		vk::CommandBuffer* m_commandBuffers;

		//Semaphores
		VkSemaphore m_semaphoreImageAvailable;

		//Buffers
		struct UniformBufferObject {// definition of the uniform buffer layout
			glm::vec3 color;
		};

		UniformBufferObject m_ubo{// the host uniform buffer
			m_ubo.color = {0, 1, 0}
		};
		vk::Buffer m_uniformBuffer = vk::Buffer();// the vulkan uniform buffer

		vk::Buffer m_vertexBuffer = vk::Buffer();
		vk::Buffer m_indexBuffer = vk::Buffer();

	};
}

