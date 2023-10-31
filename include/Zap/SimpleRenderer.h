#pragma once

#include "Renderer.h"
#include "Zap.h"
#include "Window.h"

namespace Zap {
	class SimpleRenderer : public Renderer
	{
	public:
		SimpleRenderer(Window& window);
		~SimpleRenderer();

		void init();

		void render(uint32_t cam);

	private:
		vk::DescriptorPool m_descriptorPool = vk::DescriptorPool();

		vk::Shader m_vertexShader = vk::Shader();
		vk::Shader m_fragmentShader = vk::Shader();
		vk::Pipeline m_pipeline = vk::Pipeline();

		//Fences
		VkFence m_renderComplete;

		//Buffers
		struct UniformBufferObject {// definition of the uniform buffer layout
			glm::mat4 model;
			glm::mat4 view;
			glm::mat4 perspective;
			glm::vec3 color;
		};

		UniformBufferObject m_ubo{};// the host uniform buffer
		vk::Buffer m_uniformBuffer = vk::Buffer();// the vulkan uniform buffer
	};
}

