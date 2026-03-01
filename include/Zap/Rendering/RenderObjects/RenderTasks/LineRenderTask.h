#pragma once

#include "Zap/Zap.h"
#include "Zap/Rendering/RenderObjects/RenderTask.h"
#include "Zap/Rendering/RenderObjects/RenderTargets.h"
#include "Zap/Rendering/RenderObjects/DescriptorSet.h"
#include "Zap/Rendering/RenderObjects/Framebuffer.h"

namespace Zap {
	class LineVertex {
	public:
		LineVertex();
		LineVertex(glm::vec3 pos, glm::u8vec3 color);
		~LineVertex();

		glm::vec3 pos = { 0, 0, 0 };
		glm::u8vec3 color = { 255, 255, 255 };

		static uint32_t getVertexInputAttributeDescriptionCount();

		// returns an array with following attribute descriptions
		// (binding = 0)
		// [0] position: [format](R32G32B32_SFLOAT) [location](0)
		// [2] color: [format](R8G8B8_UNORM) [location](1)
		static std::vector<VkVertexInputAttributeDescription> getVertexInputAttributeDescriptions();

		static VkVertexInputBindingDescription getVertexInputBindingDescription();
	};

	class LineBuffer {
	public:
		LineBuffer(size_t size = 0);
		~LineBuffer();

		operator VkBuffer();

		void resize(size_t size);

		void map(size_t offset, void*& ptr);

		void unmap();

		// returns the number of line vertices in the buffer
		size_t getSize();

	private:
		vk::Buffer m_vertexBuffer;
	};

	class LineRenderTask : public RenderTask {
	public:
		LineRenderTask(Renderer* pRenderer, RenderTargetHandle<> target, std::initializer_list<std::weak_ptr<LineBuffer>> lineBuffers);
		~LineRenderTask();

		void updateCamera(Actor camera);

	private:
		struct UBO {
			glm::mat4 perspective;
			glm::mat4 view;
		};
		vk::Buffer m_uniformBuffer;

		std::vector<std::weak_ptr<LineBuffer>> m_lineBuffers;

		RenderTargetHandle<> m_target;
		RenderTargetHandle<RenderTargetImage> m_depthTarget;

		DescriptorSetHandle<GenericDescriptorSet> m_descriptorSet;

		vk::RenderPass m_renderPass;

		FramebufferHandle m_framebuffer;

		vk::Shader m_vertexShader;
		vk::Shader m_fragmentShader;

		vk::Pipeline m_pipeline;

		virtual void init(const LayoutTransitionHelper& layoutTransitionHelper) override;

		virtual void destroy() override;

		virtual void recordCommands(const vk::CommandBuffer* cmd) override;

		virtual void addDescriptorPoolSizes(DescriptorPoolSizeList& poolSizes) override;

		virtual TaskLayoutTransitions getLayoutTransitions() override;

		VkViewport getViewport();
		VkRect2D getScissor();
	};
}