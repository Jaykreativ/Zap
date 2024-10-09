#pragma once

#include "Zap/Zap.h"
#include "Zap/Rendering/RenderTaskTemplate.h"
#include "Zap/Scene/Actor.h"

namespace Zap {
	class DebugRenderVertex {
	public:
		DebugRenderVertex();
		DebugRenderVertex(glm::vec3 pos, glm::u8vec3 color);
		~DebugRenderVertex();

		glm::vec3 pos = {0, 0, 0};
		glm::u8vec3 color = {255, 255, 255};

		static uint32_t getVertexInputAttributeDescriptionCount();

		// returns an array with following attribute descriptions
		// (binding = 0)
		// [0] position: [format](R32G32B32_SFLOAT) [location](0)
		// [2] color: [format](R8G8B8_UNORM) [location](1)
		static std::vector<VkVertexInputAttributeDescription> getVertexInputAttributeDescriptions();

		static VkVertexInputBindingDescription getVertexInputBindingDescription();
	};

	class DebugRenderTask : public Zap::RenderTaskTemplate {
	public:
		DebugRenderTask();
		~DebugRenderTask();

		void updateCamera(Zap::Actor cam);

		void delLineVertexBuffers();

		void addLineVertexBuffer(vk::Buffer* pVertexBuffer);

	private:
		struct UBO {
			glm::mat4 perspective;
			glm::mat4 view;
		};
		vk::Buffer m_uniformBuffer;

		std::vector<vk::Buffer*> m_lineVertexBuffers = {};

		vk::Image m_depthImage;

		vk::DescriptorPool m_descriptorPool;
		vk::DescriptorSet m_descriptorSet;

		vk::RenderPass m_renderPass;

		std::vector<vk::Framebuffer> m_framebuffers;

		vk::Shader m_vertexShader;
		vk::Shader m_fragmentShader;

		vk::Pipeline m_pipeline;

		VkViewport m_viewport = {};
		VkRect2D m_scissor = {};

		void init(uint32_t width, uint32_t height, uint32_t imageCount);

		void initTargetDependencies(uint32_t width, uint32_t height, uint32_t imageCount, vk::Image* pTarget, uint32_t imageIndex);

		void resize(uint32_t width, uint32_t height, uint32_t imageCount);

		void resizeTargetDependencies(uint32_t width, uint32_t height, uint32_t imageCount, vk::Image* pTarget, uint32_t imageIndex);

		void destroy();

		void beforeRender(vk::Image* pTarget, uint32_t imageIndex);

		void afterRender(vk::Image* pTarget, uint32_t imageIndex);

		void recordCommands(const vk::CommandBuffer* cmd, vk::Image* pTarget, uint32_t imageIndex);
	};
}