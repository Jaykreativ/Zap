#pragma once

#include "Zap/Zap.h"
#include "Zap/Rendering/RenderTemplate.h"
#include "glm.hpp"

namespace Zap {
	class PBRenderer : public RenderTemplate
	{
	public:
		PBRenderer(Renderer& renderer);
		~PBRenderer();

		void updateBuffers(uint32_t camera);

		void setViewport(uint32_t width, uint32_t height, uint32_t x, uint32_t y);

	private:
		bool m_isInit = false;

		Renderer& m_renderer;

		VkViewport m_viewport;
		VkRect2D m_scissor;

		vk::RenderPass m_renderPass = vk::RenderPass();

		vk::Image m_depthImage;

		std::vector<vk::Framebuffer> m_framebuffers;

		vk::DescriptorPool m_descriptorPool = vk::DescriptorPool();

		vk::Shader m_vertexShader = vk::Shader();
		vk::Shader m_fragmentShader = vk::Shader();
		vk::Pipeline m_pipeline = vk::Pipeline();

		//Semaphores
		VkSemaphore m_semaphoreRenderComplete;

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

		struct PerMeshData {
			glm::mat4 transform;
			glm::mat4 normalTransform;
			glm::vec4 color;
		};

		vk::Buffer m_perMeshBuffer;

		void init();

		void recordCommands(const vk::CommandBuffer* cmd, uint32_t imageIndex);
	};
}

