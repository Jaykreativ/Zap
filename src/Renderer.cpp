#include "Renderer.h"

namespace Zap {
	Renderer::Renderer(Window& window)
		: m_window(window)
	{}

	Renderer::~Renderer() {
		if (!m_isInit) return;
		m_isInit = false;

		vk::allQueuesWaitIdle();

		vk::destroyFence(m_renderComplete);

		for (VisibleActor* actor : m_actors) actor->getModel()->~Model();

		m_pipeline.~Pipeline();
		m_fragmentShader.~Shader();
		m_vertexShader.~Shader();
		m_uniformBuffer.~Buffer();

	}

	void Renderer::init() {
		if (m_isInit) return;
		m_isInit = true;

		m_uniformBuffer = vk::Buffer(sizeof(UniformBufferObject), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
		m_uniformBuffer.init(); m_uniformBuffer.allocate(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

		/*DescriptorPool*/ {
		VkDescriptorBufferInfo uniformBufferInfo;
		uniformBufferInfo.buffer = m_uniformBuffer;
		uniformBufferInfo.offset = 0;
		uniformBufferInfo.range = m_uniformBuffer.getSize();

		vk::Descriptor uniformBufferDescriptor{};
		uniformBufferDescriptor.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		uniformBufferDescriptor.stages = VK_SHADER_STAGE_VERTEX_BIT;
		uniformBufferDescriptor.binding = 0;
		uniformBufferDescriptor.pBufferInfo = &uniformBufferInfo;

		m_descriptorPool.addDescriptorSet();
		m_descriptorPool.addDescriptor(uniformBufferDescriptor, 0);
		}
		m_descriptorPool.update();

		/*Shader*/ {
		m_vertexShader.setStage(VK_SHADER_STAGE_VERTEX_BIT);
		m_vertexShader.setPath("shader.vert.spv");

		m_fragmentShader.setStage(VK_SHADER_STAGE_FRAGMENT_BIT);
		m_fragmentShader.setPath("shader.frag.spv");
		}
		m_vertexShader.init();
		m_fragmentShader.init();

		/*Pipeline*/
		m_pipeline.addShader(m_vertexShader.getShaderStage());
		m_pipeline.addShader(m_fragmentShader.getShaderStage());

		m_pipeline.addDescriptorSetLayout(m_descriptorPool.getVkDescriptorSetLayout(0));
		for (auto attributeDescription : Vertex::getVertexInputAttributeDescriptions()) {
			m_pipeline.addVertexInputAttrubuteDescription(attributeDescription);
		}
		m_pipeline.addVertexInputBindingDescription(Vertex::getVertexInputBindingDescription());
		m_pipeline.addViewport(m_viewport);
		m_pipeline.addScissor(m_scissor);
		m_pipeline.setRenderPass(*m_window.getRenderPass());
		m_pipeline.enableDepthTest();
		m_pipeline.init();

		for (VisibleActor* actor : m_actors) {
			actor->getModel()->init(m_window.getSwapchain()->getImageCount());
			actor->getModel()->recordCommandBuffers(*m_window.getRenderPass(), m_window.getFramebufferPtr(), m_scissor, m_pipeline, m_descriptorPool.getVkDescriptorSet(0));
		}

		vk::createFence(&m_renderComplete);
	}

	void Renderer::render(Camera* cam){
		if (glfwGetWindowAttrib(m_window.getGLFWwindow(), GLFW_ICONIFIED)) return;

		m_window.clear();

		for (VisibleActor* actor : m_actors) {
			m_ubo.model = actor->getTransform();
			m_ubo.view = cam->getView();
			m_ubo.perspective = cam->getPerspective(m_viewport.width / m_viewport.height);
			m_ubo.color = actor->m_color;

			void* rawData; m_uniformBuffer.map(&rawData);
			memcpy(rawData, &m_ubo, sizeof(UniformBufferObject));
			m_uniformBuffer.unmap();

			actor->getModel()->getCommandBuffer(m_window.getCurrentImageIndex())->submit(m_renderComplete);
			vk::waitForFence(m_renderComplete);
		}
	}

	uint32_t highestIndex = 0;
	void Renderer::addActor(VisibleActor& actor) {
		m_actors.push_back(&actor);
	}

	void Renderer::setViewport(uint32_t width, uint32_t height, uint32_t x, uint32_t y){
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