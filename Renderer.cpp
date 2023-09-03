#include "Renderer.h"
#include "Vertex.h"

namespace Zap {
	Renderer::Renderer(Window& window)
		: m_window(window)
	{}

	Renderer::~Renderer() {
		m_swapchain.~Swapchain();
		m_surface.~Surface();
		m_renderPass.~RenderPass();
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

		vk::Shader vertexShader = vk::Shader();
		vertexShader.setStage(VK_SHADER_STAGE_VERTEX_BIT);
		vertexShader.setPath();

		vk::Shader fragmentShader = vk::Shader();
		fragmentShader.setStage(VK_SHADER_STAGE_FRAGMENT_BIT);
		fragmentShader.setPath();

		m_pipeline.addShader(vertexShader);
		m_pipeline.addShader(fragmentShader);

		m_pipeline.addDescriptorSetLayout(m_descriptorPool.getDescriptorSetLayout(0));
		for (auto attributeDescription : Vertex::getVertexInputAttributeDescriptions()) {
			m_pipeline.addVertexInputAttrubuteDescription(attributeDescription);
		}
		m_pipeline.addVertexInputBindingDescription(Vertex::getVertexInputBindingDescription());
		m_pipeline.addViewport(m_viewport);
		m_pipeline.addScissor(m_scissor);
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