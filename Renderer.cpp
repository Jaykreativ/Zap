#include "Renderer.h"
#include "Vertex.h"
#include "Window.h"

namespace Zap {
	Renderer::Renderer(Window& window)
		: m_window(window)
	{}

	Renderer::~Renderer() {
		m_swapchain.~Swapchain();
		m_surface.~Surface();
		for (vk::Framebuffer framebuffer : m_framebuffers) framebuffer.~Framebuffer();
		m_renderPass.~RenderPass();
		m_vertexShader.~Shader();
		m_fragmentShader.~Shader();
		m_pipeline.~Pipeline();

		vk::destroySemaphore(m_semaphoreImageAvailable);
	}

	void Renderer::recordCommandBuffer(vk::CommandBuffer& commandBuffer, uint32_t index) {
		commandBuffer.begin(VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT);

		VkRenderPassBeginInfo renderPassBeginInfo;
		renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassBeginInfo.pNext = nullptr;
		renderPassBeginInfo.renderPass = m_renderPass.getVkRenderPass();
		renderPassBeginInfo.framebuffer = m_framebuffers[index].getVkFramebuffer();
		renderPassBeginInfo.renderArea = m_scissor;
		std::vector<VkClearValue> clearValues{
			{ 0.1, 0.1, 0.1, 1 }
		};
		renderPassBeginInfo.clearValueCount = clearValues.size();
		renderPassBeginInfo.pClearValues = clearValues.data();
		vkCmdBeginRenderPass(commandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline.getVkPipeline());

		VkDeviceSize offsets[] = { 0 };
		vkCmdBindVertexBuffers(commandBuffer, 0, 1, &m_vertexBuffer.getVkBuffer(), offsets);
		vkCmdBindIndexBuffer(commandBuffer, m_indexBuffer.getVkBuffer(), 0, VK_INDEX_TYPE_UINT32);

		vkCmdDrawIndexed(commandBuffer, indexArray.size(), 0, 0, 0, 0);

		vkCmdEndRenderPass(commandBuffer);

		commandBuffer.end();
	}

	void Renderer::init() {
		/*Surface*/ {
			m_surface.setGLFWwindow(m_window.getGLFWwindow());
		}
		m_surface.init();

		/*Swapchain*/ {
			m_swapchain.setWidth(m_window.getWidth());
			m_swapchain.setHeight(m_window.getHeight());
			m_swapchain.setPresentMode(VK_PRESENT_MODE_MAILBOX_KHR);
			m_swapchain.setSurface(m_surface);
		}
		m_swapchain.init();

		m_uniformBuffer = vk::Buffer(sizeof(UniformBufferObject), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
		m_uniformBuffer.init(); m_uniformBuffer.allocate(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

		/*DescriptorPool*/ {
		VkDescriptorBufferInfo uniformBufferInfo;
		uniformBufferInfo.buffer = m_uniformBuffer.getVkBuffer();
		uniformBufferInfo.offset = 0;
		uniformBufferInfo.range = m_uniformBuffer.getSize();

		vk::Descriptor uniformBufferDescriptor{};
		uniformBufferDescriptor.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		uniformBufferDescriptor.stages = VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT;
		uniformBufferDescriptor.binding = 0;
		uniformBufferDescriptor.pBufferInfo = &uniformBufferInfo;

		m_descriptorPool.addDescriptorSet();
		m_descriptorPool.addDescriptor(uniformBufferDescriptor, 0);
		}
		m_descriptorPool.init();

		/*RenderPass*/ {
			VkAttachmentDescription colorAttachment;
			colorAttachment.flags = 0;
			colorAttachment.format = Zap::GlobalSettings::getColorFormat();
			colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
			colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
			colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
			colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

			m_renderPass.addAttachmentDescription(colorAttachment);

			VkAttachmentReference* pColorAttachmentReference;
			{
				VkAttachmentReference tmp;
				tmp.attachment = 0;
				tmp.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
				pColorAttachmentReference = &tmp;

				m_renderPass.addAttachmentReference(&pColorAttachmentReference);
			}

			VkSubpassDescription subpassDescription;
			subpassDescription.flags = 0;
			subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
			subpassDescription.inputAttachmentCount = 0;
			subpassDescription.pInputAttachments = nullptr;
			subpassDescription.colorAttachmentCount = 1;
			subpassDescription.pColorAttachments = pColorAttachmentReference;
			subpassDescription.pResolveAttachments = nullptr;
			subpassDescription.pDepthStencilAttachment = nullptr;
			subpassDescription.preserveAttachmentCount = 0;
			subpassDescription.pPreserveAttachments = nullptr;

			m_renderPass.addSubpassDescription(subpassDescription);

			VkSubpassDependency subpassDependency;
			subpassDependency.srcSubpass = VK_SUBPASS_EXTERNAL;
			subpassDependency.dstSubpass = 0;
			subpassDependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			subpassDependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			subpassDependency.srcAccessMask = 0;
			subpassDependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
			subpassDependency.dependencyFlags = 0;

			m_renderPass.addSubpassDependency(subpassDependency);
		}
		m_renderPass.init();

		/*Framebuffer*/ {
		m_framebuffers.resize(m_swapchain.getImageCount());
		for (int i = 0; i < m_swapchain.getImageCount(); i++) {
			m_framebuffers[i].setWidth(m_viewport.width);
			m_framebuffers[i].setHeight(m_viewport.height);
			m_framebuffers[i].addAttachment(m_swapchain.getImageView(i));
			m_framebuffers[i].setRenderPass(m_renderPass);
			m_framebuffers[i].init();
		}
		}

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

		m_pipeline.addDescriptorSetLayout(m_descriptorPool.getDescriptorSetLayout(0));
		for (auto attributeDescription : Vertex::getVertexInputAttributeDescriptions()) {
			m_pipeline.addVertexInputAttrubuteDescription(attributeDescription);
		}
		m_pipeline.addVertexInputBindingDescription(Vertex::getVertexInputBindingDescription());
		m_pipeline.addViewport(m_viewport);
		m_pipeline.addScissor(m_scissor);
		m_pipeline.setRenderPass(m_renderPass);
		}
		m_pipeline.init();

		m_vertexBuffer.resize(vertexArray.size() * sizeof(Vertex)); m_vertexBuffer.setUsage(VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);
		m_vertexBuffer.init(); m_vertexBuffer.allocate(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
		m_vertexBuffer.uploadData(m_vertexBuffer.getSize(), vertexArray.data());

		m_indexBuffer.resize(indexArray.size() * sizeof(uint32_t)); m_indexBuffer.setUsage(VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);
		m_indexBuffer.init(); m_indexBuffer.allocate(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
		m_indexBuffer.uploadData(m_indexBuffer.getSize(), indexArray.data());

		vk::createSemaphore(&m_semaphoreImageAvailable);

		m_commandBufferCount = m_swapchain.getImageCount();
		m_commandBuffers = new vk::CommandBuffer[m_commandBufferCount];
		for (int i = 0; i < m_swapchain.getImageCount(); i++) {
			m_commandBuffers[i].allocate();
			m_commandBuffers[i].addWaitSemaphore(m_semaphoreImageAvailable, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);
			recordCommandBuffer(m_commandBuffers[i], i);
		}
	}

	void Renderer::render() {
		uint32_t imageIndex = 0;
		vk::acquireNextImage(m_swapchain, m_semaphoreImageAvailable, VK_NULL_HANDLE, &imageIndex);

		VkQueue queue;
		m_commandBuffers[imageIndex].submit(&queue);

		vk::queuePresent(queue, m_swapchain, imageIndex);
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