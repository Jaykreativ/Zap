#pragma once

#include "Zap/Zap.h"
#include "Zap/Renderer.h"
#include "Zap/Window.h"

namespace Zap {
	class PBRenderer : public Renderer
	{
	public:
		PBRenderer(Window& window);
		~PBRenderer();

		void init();

		void recordCommandBuffers();

		void render(uint32_t cam);

	private:
		vk::DescriptorPool m_descriptorPool = vk::DescriptorPool();

		vk::Shader m_vertexShader = vk::Shader();
		vk::Shader m_fragmentShader = vk::Shader();
		vk::Pipeline m_pipeline = vk::Pipeline();

		//Semaphores
		VkSemaphore m_semaphoreRenderComplete;

		//Fences
		VkFence m_renderComplete;

		//Buffers
		struct UniformBufferObject {// definition of the uniform buffer layout
			glm::mat4 model;
			glm::mat4 modelNormal;
			glm::mat4 view;
			glm::mat4 perspective;
			alignas(16) glm::vec3 color;
			alignas(4) uint32_t lightCount;
		};

		UniformBufferObject m_ubo{};// the host uniform buffer
		vk::Buffer m_uniformBuffer = vk::Buffer();// the vulkan uniform buffer

		struct LightData {
			alignas(16) glm::vec3 pos;
			alignas(16) glm::vec3 color;
		};

		vk::Buffer m_lightBuffer = vk::Buffer();
	};
}

