#include "Zap/Rendering/DebugRenderTask.h"

namespace Zap {
	/* DebugRenderVertex */

	DebugRenderVertex::DebugRenderVertex() {}
	DebugRenderVertex::DebugRenderVertex(glm::vec3 pos, glm::u8vec3 color)
		: pos(pos), color(color)
	{}
	DebugRenderVertex::~DebugRenderVertex() {}

	uint32_t DebugRenderVertex::getVertexInputAttributeDescriptionCount() {
		return 2;
	}

	std::vector<VkVertexInputAttributeDescription> DebugRenderVertex::getVertexInputAttributeDescriptions() {
		VkVertexInputAttributeDescription posAttributeDescription{};
		posAttributeDescription.location = 0;
		posAttributeDescription.binding = 0;
		posAttributeDescription.format = VK_FORMAT_R32G32B32_SFLOAT;
		posAttributeDescription.offset = offsetof(DebugRenderVertex, pos);

		VkVertexInputAttributeDescription colorAttributeDescription{};
		colorAttributeDescription.location = 1;
		colorAttributeDescription.binding = 0;
		colorAttributeDescription.format = VK_FORMAT_R8G8B8_UNORM;
		colorAttributeDescription.offset = offsetof(DebugRenderVertex, color);

		std::vector<VkVertexInputAttributeDescription> attributeDescriptions = {
			posAttributeDescription,
			colorAttributeDescription
		};
		return attributeDescriptions;
	}

	VkVertexInputBindingDescription DebugRenderVertex::getVertexInputBindingDescription() {
		VkVertexInputBindingDescription bindingDescription;
		bindingDescription.binding = 0;
		bindingDescription.stride = sizeof(DebugRenderVertex);
		bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
		return bindingDescription;
	}

	/* DebugRenderTask */

	DebugRenderTask::DebugRenderTask() {}

	DebugRenderTask::~DebugRenderTask() {}

	void DebugRenderTask::updateCamera(Zap::Actor cam) {
		void* rawData;
		m_uniformBuffer.map(&rawData);
		UBO* data = (UBO*)rawData;
		data->perspective = cam.cmpCamera_getPerspective(m_viewport.width / m_viewport.height);
		data->view = cam.cmpCamera_getView();
		m_uniformBuffer.unmap();
	}

	void DebugRenderTask::delLineVertexBuffers() {
		m_lineVertexBuffers.clear();
	}

	void DebugRenderTask::addLineVertexBuffer(vk::Buffer* pVertexBuffer) {
		m_lineVertexBuffers.push_back(pVertexBuffer);
	}

	void DebugRenderTask::init(uint32_t width, uint32_t height, uint32_t imageCount) {
		m_viewport.x = 0;
		m_viewport.y = 0;
		m_viewport.width = width;
		m_viewport.height = height;
		m_viewport.minDepth = 0;
		m_viewport.maxDepth = 1;

		m_scissor.extent.width = width;
		m_scissor.extent.height = height;
		m_scissor.offset.x = 0;
		m_scissor.offset.y = 0;

		m_depthImage = vk::Image();
		m_depthImage.setAspect(VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT);
		m_depthImage.setExtent({ width, height, 1 });
		m_depthImage.setFormat(Zap::GlobalSettings::getDepthStencilFormat());
		m_depthImage.setLayout(VK_IMAGE_LAYOUT_UNDEFINED);
		m_depthImage.setType(VK_IMAGE_TYPE_2D);
		m_depthImage.setUsage(VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT);
		m_depthImage.init();
		m_depthImage.allocate(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
		m_depthImage.initView();

		m_depthImage.changeLayout(VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT);

		m_uniformBuffer = vk::Buffer(sizeof(UBO), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
		m_uniformBuffer.init();
		m_uniformBuffer.allocate(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

		vk::DescriptorBufferInfo uniformBufferInfo{};
		uniformBufferInfo.pBuffer = &m_uniformBuffer;
		uniformBufferInfo.offset = 0;
		uniformBufferInfo.range = m_uniformBuffer.getSize();

		vk::Descriptor uniformBufferDescriptor{};
		uniformBufferDescriptor.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		uniformBufferDescriptor.stages = VK_SHADER_STAGE_VERTEX_BIT;
		uniformBufferDescriptor.binding = 0;
		uniformBufferDescriptor.bufferInfos = { uniformBufferInfo };

		m_descriptorSet.addDescriptor(uniformBufferDescriptor);

		m_descriptorPool.addDescriptorSet(m_descriptorSet);
		m_descriptorPool.init();

		m_descriptorSet.init();
		m_descriptorSet.allocate();
		m_descriptorSet.update();

		m_renderPass = vk::RenderPass();
		{
			VkAttachmentDescription targetAttachmentDescription{};
			targetAttachmentDescription.flags = 0;
			targetAttachmentDescription.format = Zap::GlobalSettings::getColorFormat();
			targetAttachmentDescription.samples = VK_SAMPLE_COUNT_1_BIT;
			targetAttachmentDescription.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
			targetAttachmentDescription.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
			targetAttachmentDescription.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			targetAttachmentDescription.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			targetAttachmentDescription.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
			targetAttachmentDescription.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

			m_renderPass.addAttachmentDescription(targetAttachmentDescription);

			VkAttachmentDescription depthStencilAttachmentDescription{};
			depthStencilAttachmentDescription.flags = 0;
			depthStencilAttachmentDescription.format = m_depthImage.getFormat();
			depthStencilAttachmentDescription.samples = VK_SAMPLE_COUNT_1_BIT;
			depthStencilAttachmentDescription.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
			depthStencilAttachmentDescription.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
			depthStencilAttachmentDescription.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			depthStencilAttachmentDescription.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			depthStencilAttachmentDescription.initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
			depthStencilAttachmentDescription.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

			m_renderPass.addAttachmentDescription(depthStencilAttachmentDescription);

			VkAttachmentReference* pTargetAttachmentReference;
			{
				VkAttachmentReference tmp;
				tmp.attachment = 0;
				tmp.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
				pTargetAttachmentReference = &tmp;

				m_renderPass.addAttachmentReference(&pTargetAttachmentReference);
			}

			VkAttachmentReference* pDepthStencilAttachmentReference;
			{
				VkAttachmentReference tmp;
				tmp.attachment = 1;
				tmp.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
				pDepthStencilAttachmentReference = &tmp;

				m_renderPass.addAttachmentReference(&pDepthStencilAttachmentReference);
			}

			VkSubpassDescription subpassDescription{};
			subpassDescription.flags = 0;
			subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
			subpassDescription.inputAttachmentCount = 0;
			subpassDescription.pInputAttachments = nullptr;
			subpassDescription.colorAttachmentCount = 1;
			subpassDescription.pColorAttachments = pTargetAttachmentReference;
			subpassDescription.pResolveAttachments = nullptr;
			subpassDescription.pDepthStencilAttachment = pDepthStencilAttachmentReference;
			subpassDescription.preserveAttachmentCount = 0;
			subpassDescription.pPreserveAttachments = nullptr;

			m_renderPass.addSubpassDescription(subpassDescription);

			VkSubpassDependency subpassDependency{};
			subpassDependency.srcSubpass = VK_SUBPASS_EXTERNAL;
			subpassDependency.dstSubpass = 0;
			subpassDependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			subpassDependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			subpassDependency.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
			subpassDependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
			subpassDependency.dependencyFlags = 0;

			m_renderPass.addSubpassDependency(subpassDependency);

			m_renderPass.init();
		}

		m_framebuffers.resize(imageCount);
		Zap::RenderTaskTemplate::initTargetDependencies();

#ifdef _DEBUG
		static bool areShadersCompiled = false;
		if (!areShadersCompiled) {
			vk::Shader::compile("Shader/src/", { "debug.vert", "debug.frag" }, { "./" });
			areShadersCompiled = true;
		}
#endif

		m_vertexShader = vk::Shader();
		m_vertexShader.setStage(VK_SHADER_STAGE_VERTEX_BIT);
		m_vertexShader.setPath("debug.vert.spv");
		m_vertexShader.init();

		m_fragmentShader = vk::Shader();
		m_fragmentShader.setStage(VK_SHADER_STAGE_FRAGMENT_BIT);
		m_fragmentShader.setPath("debug.frag.spv");
		m_fragmentShader.init();

		m_pipeline = vk::Pipeline();
		{
			m_pipeline.addShader(m_vertexShader.getShaderStage());
			m_pipeline.addShader(m_fragmentShader.getShaderStage());

			m_pipeline.addDescriptorSetLayout(m_descriptorSet.getVkDescriptorSetLayout());
			for (uint32_t i = 0; i < DebugRenderVertex::getVertexInputAttributeDescriptionCount(); i++)
				m_pipeline.addVertexInputAttrubuteDescription(DebugRenderVertex::getVertexInputAttributeDescriptions()[i]);
			m_pipeline.addVertexInputBindingDescription(DebugRenderVertex::getVertexInputBindingDescription());
			m_pipeline.setPrimitiveTopology(VK_PRIMITIVE_TOPOLOGY_LINE_LIST);
			m_pipeline.addDynamicState(VK_DYNAMIC_STATE_VIEWPORT);
			m_pipeline.addDynamicState(VK_DYNAMIC_STATE_SCISSOR);
			m_pipeline.addViewport(m_viewport);
			m_pipeline.addScissor(m_scissor);
			m_pipeline.setRenderPass(m_renderPass);
			m_pipeline.enableDepthTest();

			m_pipeline.init();
		}
	}

	void DebugRenderTask::initTargetDependencies(uint32_t width, uint32_t height, uint32_t imageCount, vk::Image* pTarget, uint32_t imageIndex) {
		m_framebuffers[imageIndex].setWidth(width);
		m_framebuffers[imageIndex].setHeight(height);
		m_framebuffers[imageIndex].addAttachment(pTarget->getVkImageView());
		m_framebuffers[imageIndex].addAttachment(m_depthImage.getVkImageView());
		m_framebuffers[imageIndex].setRenderPass(m_renderPass);
		m_framebuffers[imageIndex].init();
	}

	void DebugRenderTask::resize(uint32_t width, uint32_t height, uint32_t imageCount) {
		/*Viewport & Scissor*/
		m_viewport.width = width;
		m_viewport.height = height;

		m_scissor.extent.width = width;
		m_scissor.extent.height = height;

		m_depthImage.resize(width, height);

		m_descriptorSet.update();

		RenderTaskTemplate::resizeTargetDependencies();

	}

	void DebugRenderTask::resizeTargetDependencies(uint32_t width, uint32_t height, uint32_t imageCount, vk::Image* pTarget, uint32_t imageIndex) {
		m_framebuffers[imageIndex].setWidth(width);
		m_framebuffers[imageIndex].setHeight(height);
		m_framebuffers[imageIndex].delAttachment(0);
		m_framebuffers[imageIndex].delAttachment(0);
		m_framebuffers[imageIndex].addAttachment(pTarget->getVkImageView());
		m_framebuffers[imageIndex].addAttachment(m_depthImage.getVkImageView());
		m_framebuffers[imageIndex].update();
	}

	void DebugRenderTask::destroy() {
		m_pipeline.destroy();
		m_vertexShader.destroy();
		m_fragmentShader.destroy();
		for (auto& framebuffer : m_framebuffers)
			framebuffer.destroy();
		m_renderPass.destroy();
		m_descriptorSet.destroy();
		m_descriptorPool.destroy();
		m_uniformBuffer.destroy();
		m_depthImage.destroy();
	}

	void DebugRenderTask::beforeRender(vk::Image* pTarget, uint32_t imageIndex) {}

	void DebugRenderTask::afterRender(vk::Image* pTarget, uint32_t imageIndex) {}

	void DebugRenderTask::recordCommands(const vk::CommandBuffer* cmd, vk::Image* pTarget, uint32_t imageIndex) {
		VkRenderPassBeginInfo renderPassBeginInfo{ VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO, nullptr };
		renderPassBeginInfo.renderPass = m_renderPass;
		renderPassBeginInfo.framebuffer = m_framebuffers[imageIndex];
		renderPassBeginInfo.renderArea = m_scissor;

		VkClearValue targetClearValue = { 0, 0, 0, 0 };
		VkClearValue depthClearValue = { 1 };
		std::vector<VkClearValue> clearValues = {
			targetClearValue,
			depthClearValue,
		};

		renderPassBeginInfo.clearValueCount = clearValues.size();
		renderPassBeginInfo.pClearValues = clearValues.data();

		vkCmdBeginRenderPass(*cmd, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

		vkCmdBindPipeline(*cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline);

		vkCmdSetViewport(*cmd, 0, 1, &m_viewport);
		vkCmdSetScissor(*cmd, 0, 1, &m_scissor);

		VkDescriptorSet boundSets[] = { m_descriptorSet };
		vkCmdBindDescriptorSets(*cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline.getVkPipelineLayout(), 0, 1, boundSets, 0, nullptr);

		for (auto vertexBuffer : m_lineVertexBuffers) {
			VkBuffer buffer = *vertexBuffer;
			VkDeviceSize offsets[] = { 0 };
			vkCmdBindVertexBuffers(*cmd, 0, 1, &buffer, offsets);

			vkCmdDraw(*cmd, vertexBuffer->getSize() / sizeof(DebugRenderVertex), 1, 0, 0);
		}

		vkCmdEndRenderPass(*cmd);
	}

}