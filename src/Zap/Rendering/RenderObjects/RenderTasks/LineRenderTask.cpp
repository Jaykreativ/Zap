#include "Zap/Rendering/RenderObjects/RenderTasks/LineRenderTask.h"

#include "Zap/Scene/Actor.h"
#include "Zap/Rendering/Renderer.h"

namespace Zap {
	/* LineVertex */
	LineVertex::LineVertex() {}
	LineVertex::LineVertex(glm::vec3 pos, glm::u8vec3 color)
		: pos(pos), color(color)
	{}
	LineVertex::~LineVertex() {}

	uint32_t LineVertex::getVertexInputAttributeDescriptionCount() {
		return 2;
	}

	std::vector<VkVertexInputAttributeDescription> LineVertex::getVertexInputAttributeDescriptions() {
		VkVertexInputAttributeDescription posAttributeDescription{};
		posAttributeDescription.location = 0;
		posAttributeDescription.binding = 0;
		posAttributeDescription.format = VK_FORMAT_R32G32B32_SFLOAT;
		posAttributeDescription.offset = offsetof(LineVertex, pos);

		VkVertexInputAttributeDescription colorAttributeDescription{};
		colorAttributeDescription.location = 1;
		colorAttributeDescription.binding = 0;
		colorAttributeDescription.format = VK_FORMAT_R8G8B8_UNORM;
		colorAttributeDescription.offset = offsetof(LineVertex, color);

		std::vector<VkVertexInputAttributeDescription> attributeDescriptions = {
			posAttributeDescription,
			colorAttributeDescription
		};
		return attributeDescriptions;
	}

	VkVertexInputBindingDescription LineVertex::getVertexInputBindingDescription() {
		VkVertexInputBindingDescription bindingDescription;
		bindingDescription.binding = 0;
		bindingDescription.stride = sizeof(LineVertex);
		bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
		return bindingDescription;
	}

	/* LineBuffer */
	LineBuffer::LineBuffer(size_t size) {
		m_vertexBuffer = vk::Buffer(size, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
		m_vertexBuffer.init();
		m_vertexBuffer.allocate(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
	}

	LineBuffer::~LineBuffer() {
		m_vertexBuffer.destroy();
	}

	LineBuffer::operator VkBuffer() {
		return m_vertexBuffer;
	}

	void LineBuffer::resize(size_t size) {
		m_vertexBuffer.resize(size);
	}

	void LineBuffer::map(size_t offset, void*& ptr) {
		m_vertexBuffer.map(offset, &ptr);
	}

	void LineBuffer::unmap() {
		m_vertexBuffer.unmap();
	}

	size_t LineBuffer::getSize() {
		return m_vertexBuffer.getSize() / sizeof(LineVertex);
	}

	/* LineRenderTask */
	LineRenderTask::LineRenderTask(Renderer* pRenderer, RenderTargetHandle<> target, std::initializer_list<std::weak_ptr<LineBuffer>> lineBuffers)
		: RenderTask(pRenderer), m_target(target), m_lineBuffers(lineBuffers)
	{}
	LineRenderTask::~LineRenderTask() {}

	void LineRenderTask::updateCamera(Actor camera) {
		void* rawData;
		m_uniformBuffer.map(&rawData);
		UBO* data = (UBO*)rawData;
		data->perspective = camera.cmpCamera_getPerspective(m_target->getExtent().width / (float)m_target->getExtent().height);
		data->view = camera.cmpCamera_getView();
		m_uniformBuffer.unmap();
	}

	void LineRenderTask::init(const LayoutTransitionHelper& layoutTransitionHelper) {
		/*UniformBuffer*/
		m_uniformBuffer = vk::Buffer(sizeof(UBO), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
		m_uniformBuffer.init();
		m_uniformBuffer.allocate(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

		/*DescriptorSet*/
		m_descriptorSet = m_pRenderer->createDescriptorSet<GenericDescriptorSet>();

		DescriptorSetBinding uniformBufferBinding( // binding #0
			VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
			VK_SHADER_STAGE_VERTEX_BIT
		);
		VkDescriptorBufferInfo uniformBufferInfo{ m_uniformBuffer, 0, m_uniformBuffer.getSize() };

		m_descriptorSet->addBinding(uniformBufferBinding);
		m_descriptorSet->createLayout();
		m_descriptorSet->allocate();
		VkWriteDescriptorSet descriptorSetWrite = m_descriptorSet->writeBuffer(uniformBufferInfo, 0);
		m_descriptorSet->write(1, &descriptorSetWrite);

		/*DepthTarget*/
		m_depthTarget = m_pRenderer->createRenderTarget<RenderTargetImage>();
		m_depthTarget->setAspect(VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT);
		m_depthTarget->setFormat(Zap::GlobalSettings::getDepthStencilFormat());
		m_depthTarget->setUsage(VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT);
		m_depthTarget->setInitialLayout(VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
		m_depthTarget->setFinalLayout(VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
		m_depthTarget->init(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

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
			targetAttachmentDescription.initialLayout = layoutTransitionHelper.getTransition(m_target).oldLayout;
			targetAttachmentDescription.finalLayout = layoutTransitionHelper.getTransition(m_target).newLayout;

			m_renderPass.addAttachmentDescription(targetAttachmentDescription);

			VkAttachmentDescription depthStencilAttachmentDescription{};
			depthStencilAttachmentDescription.flags = 0;
			depthStencilAttachmentDescription.format = Zap::GlobalSettings::getDepthStencilFormat();
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

		m_framebuffer = m_pRenderer->createFramebuffer(m_renderPass, {m_target, m_depthTarget});

		m_vertexShader.setStage(VK_SHADER_STAGE_VERTEX_BIT);
		m_vertexShader.setPath("line.vert.spv");
		m_vertexShader.init();

		m_fragmentShader.setStage(VK_SHADER_STAGE_FRAGMENT_BIT);
		m_fragmentShader.setPath("line.frag.spv");
		m_fragmentShader.init();

		m_pipeline.addShader(m_vertexShader.getShaderStage());
		m_pipeline.addShader(m_fragmentShader.getShaderStage());

		m_pipeline.addDescriptorSetLayout(m_descriptorSet->getLayout());
		for (uint32_t i = 0; i < LineVertex::getVertexInputAttributeDescriptionCount(); i++)
			m_pipeline.addVertexInputAttrubuteDescription(LineVertex::getVertexInputAttributeDescriptions()[i]);
		m_pipeline.addVertexInputBindingDescription(LineVertex::getVertexInputBindingDescription());
		m_pipeline.setPrimitiveTopology(VK_PRIMITIVE_TOPOLOGY_LINE_LIST);
		m_pipeline.addDynamicState(VK_DYNAMIC_STATE_VIEWPORT);
		m_pipeline.addDynamicState(VK_DYNAMIC_STATE_SCISSOR);
		m_pipeline.addViewport(getViewport());
		m_pipeline.addScissor(getScissor());
		m_pipeline.setRenderPass(m_renderPass);
		m_pipeline.enableDepthTest();

		m_pipeline.init();
	}

	void LineRenderTask::destroy() {
		m_pipeline.destroy();
		m_vertexShader.destroy();
		m_fragmentShader.destroy();
		m_pRenderer->destroyFramebuffer(m_framebuffer);
		m_renderPass.destroy();
		m_pRenderer->destroyRenderTarget(m_depthTarget);
		m_pRenderer->destroyDescriptorSet(m_descriptorSet);
		m_uniformBuffer.destroy();
	}

	void LineRenderTask::recordCommands(const vk::CommandBuffer* cmd) {
		VkRenderPassBeginInfo renderPassBeginInfo{ VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO, nullptr };
		renderPassBeginInfo.renderPass = m_renderPass;
		renderPassBeginInfo.framebuffer = *m_framebuffer.get();
		renderPassBeginInfo.renderArea = getScissor();

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

		vkCmdSetViewport(*cmd, 0, 1, &getViewport());
		vkCmdSetScissor(*cmd, 0, 1, &getScissor());

		VkDescriptorSet boundSets[] = { m_descriptorSet };
		vkCmdBindDescriptorSets(*cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline.getVkPipelineLayout(), 0, 1, boundSets, 0, nullptr);

		for (auto it = m_lineBuffers.begin(); it != m_lineBuffers.end(); it++) {
			if (auto spLineBuffer = it->lock()) { // lock weak ptr for access
				VkBuffer buffer = *spLineBuffer.get();
				VkDeviceSize offsets[] = { 0 };
				vkCmdBindVertexBuffers(*cmd, 0, 1, &buffer, offsets);

				vkCmdDraw(*cmd, spLineBuffer->getSize(), 1, 0, 0);
			}
			else {
				m_lineBuffers.erase(it); // delete weak ptr if invalid
			}
		}

		vkCmdEndRenderPass(*cmd);
	}

	void LineRenderTask::addDescriptorPoolSizes(DescriptorPoolSizeList& poolSizes) {
		poolSizes.addSets(1);
		poolSizes.addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1);
	}

	TaskLayoutTransitions LineRenderTask::getLayoutTransitions() {
		TaskLayoutTransitions  transition;
		transition.addLayout(m_target, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
		return transition;
	}

	VkViewport LineRenderTask::getViewport() {
		VkViewport viewport;
		viewport.x = 0;
		viewport.y = 0;
		viewport.width = m_target->getExtent().width;
		viewport.height = m_target->getExtent().height;
		viewport.minDepth = 0;
		viewport.maxDepth = 1;
		return viewport;
	}
	VkRect2D LineRenderTask::getScissor() {
		VkRect2D scissor;
		scissor.extent.width = m_target->getExtent().width;
		scissor.extent.height = m_target->getExtent().height;
		scissor.offset.x = 0;
		scissor.offset.y = 0;
		return scissor;
	}
}