#include "Renderer.h"
#include "Vertex.h"
#include "Window.h"

namespace Zap {
	Renderer::Renderer(Window* window)
		: m_window(window)
	{}

	Renderer::~Renderer() {
		if (!m_isInit) return;
		m_isInit = false;

		vk::allQueuesWaitIdle();

		vk::destroySemaphore(m_semaphoreImageAvailable);

		delete[] m_commandBuffers;
		m_vertexBuffer.~Buffer();
		m_indexBuffer.~Buffer();
		m_pipeline.~Pipeline();
		m_fragmentShader.~Shader();
		m_vertexShader.~Shader();
		m_uniformBuffer.~Buffer();

	}

	void Renderer::recordCommandBuffers() {
		for (int i = 0; i < m_commandBufferCount; i++) {
			vk::CommandBuffer& commandBuffer = m_commandBuffers[i];
			commandBuffer.begin(VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT);

			VkRenderPassBeginInfo renderPassBeginInfo;
			renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
			renderPassBeginInfo.pNext = nullptr;
			renderPassBeginInfo.renderPass = m_window->getRenderPass()->getVkRenderPass();
			renderPassBeginInfo.framebuffer = m_window->getFramebuffer(i)->getVkFramebuffer();
			renderPassBeginInfo.renderArea = m_scissor;
			std::vector<VkClearValue> clearValues {
				{ 0.1, 0.1, 0.1, 1 }
			};
			renderPassBeginInfo.clearValueCount = clearValues.size();
			renderPassBeginInfo.pClearValues = clearValues.data();
			vkCmdBeginRenderPass(commandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

			vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline.getVkPipeline());

			VkDeviceSize offsets[] = { 0 };
			vkCmdBindVertexBuffers(commandBuffer, 0, 1, &m_vertexBuffer.getVkBuffer(), offsets);
			vkCmdBindIndexBuffer(commandBuffer, m_indexBuffer.getVkBuffer(), 0, VK_INDEX_TYPE_UINT32);

			vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline.getPipelineLayout(), 0, 1, &Zap::getDescriptorPool().getDescriptorSet(m_descriptorSetIndex), 0, nullptr);

			vkCmdDrawIndexed(commandBuffer, indexArray.size(), 1, 0, 0, 0);

			vkCmdEndRenderPass(commandBuffer);

			commandBuffer.end();
		}
	}

	void Renderer::init() {
		if (m_isInit) return;
		m_isInit = true;

		m_uniformBuffer = vk::Buffer(sizeof(UniformBufferObject), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
		m_uniformBuffer.init(); m_uniformBuffer.allocate(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

		/*DescriptorPool*/ {
		VkDescriptorBufferInfo uniformBufferInfo;
		uniformBufferInfo.buffer = m_uniformBuffer.getVkBuffer();
		uniformBufferInfo.offset = 0;
		uniformBufferInfo.range = m_uniformBuffer.getSize();

		vk::Descriptor uniformBufferDescriptor{};
		uniformBufferDescriptor.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		uniformBufferDescriptor.stages = VK_SHADER_STAGE_VERTEX_BIT;
		uniformBufferDescriptor.binding = 0;
		uniformBufferDescriptor.pBufferInfo = &uniformBufferInfo;

		Zap::getDescriptorPool().addDescriptorSet();
		m_descriptorSetIndex = Zap::getDescriptorPool().getDescriptorSetCount() - 1;
		Zap::getDescriptorPool().addDescriptor(uniformBufferDescriptor, m_descriptorSetIndex);
		}
		Zap::getDescriptorPool().update();

		/*Shader*/ {
		m_vertexShader.setStage(VK_SHADER_STAGE_VERTEX_BIT);
		m_vertexShader.setPath("shader.vert.spv");

		m_fragmentShader.setStage(VK_SHADER_STAGE_FRAGMENT_BIT);
		m_fragmentShader.setPath("shader.frag.spv");
		}
		m_vertexShader.init();
		m_fragmentShader.init();

		/*Pipeline*/ {
		m_pipeline.addShader(m_vertexShader.getShaderStage());
		m_pipeline.addShader(m_fragmentShader.getShaderStage());

		m_pipeline.addDescriptorSetLayout(Zap::getDescriptorPool().getDescriptorSetLayout(m_descriptorSetIndex));
		for (auto attributeDescription : Vertex::getVertexInputAttributeDescriptions()) {
			m_pipeline.addVertexInputAttrubuteDescription(attributeDescription);
		}
		m_pipeline.addVertexInputBindingDescription(Vertex::getVertexInputBindingDescription());
		m_pipeline.addViewport(m_viewport);
		m_pipeline.addScissor(m_scissor);
		m_pipeline.setRenderPass(*m_window->getRenderPass());
		}
		m_pipeline.init();

		m_vertexBuffer = vk::Buffer((vertexArray.size() + 0) * sizeof(Vertex), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);
		m_vertexBuffer.init(); m_vertexBuffer.allocate(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
		m_vertexBuffer.uploadData((vertexArray.size() + 0) * sizeof(Vertex), vertexArray.data());

		m_indexBuffer.resize(indexArray.size() * sizeof(uint32_t)); m_indexBuffer.setUsage(VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);
		m_indexBuffer.init(); m_indexBuffer.allocate(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
		m_indexBuffer.uploadData(m_indexBuffer.getSize(), indexArray.data());

		vk::createSemaphore(&m_semaphoreImageAvailable);

		m_commandBufferCount = m_window->getSwapchain()->getImageCount();
		m_commandBuffers = new vk::CommandBuffer[m_commandBufferCount];
		for (int i = 0; i < m_commandBufferCount; i++) {
			m_commandBuffers[i].allocate();
			m_commandBuffers[i].addWaitSemaphore(m_semaphoreImageAvailable, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);
			recordCommandBuffers();
		}
	}

	void Renderer::render(){
		m_ubo.color = { 1, 1, 1 };

		void* rawData; m_uniformBuffer.map(&rawData);
		memcpy(rawData, &m_ubo, sizeof(UniformBufferObject));
		m_uniformBuffer.unmap();

		uint32_t imageIndex = 0;
		vk::acquireNextImage(*m_window->getSwapchain(), m_semaphoreImageAvailable, VK_NULL_HANDLE, &imageIndex);

		VkQueue queue;
		m_commandBuffers[imageIndex].submit(&queue);

		vk::queuePresent(queue, *m_window->getSwapchain(), imageIndex);
	}

	void Renderer::setViewport(uint32_t width, uint32_t height, uint32_t x, uint32_t y) {
		m_viewport.x = x;
		m_viewport.y = y;
		m_viewport.width = width;
		m_viewport.height = height;
		m_viewport.minDepth = 0;
		m_viewport.maxDepth = 1;

		m_scissor.offset.x = x;
		m_scissor.offset.y = y;
		m_scissor.extent.width = width;
		m_scissor.extent.height = height;
	}
}