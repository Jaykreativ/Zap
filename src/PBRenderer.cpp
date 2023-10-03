#include "PBRenderer.h"

namespace Zap {
	PBRenderer::PBRenderer(Window& window)
		: Renderer(window) {}
	PBRenderer::~PBRenderer() {
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

	void PBRenderer::init() {
		if (m_isInit) return;
		m_isInit = true;

		/*UniformBuffers*/
		m_uniformBuffer = vk::Buffer(sizeof(UniformBufferObject), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
		m_uniformBuffer.init(); m_uniformBuffer.allocate(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

		m_lightBuffer = vk::Buffer(sizeof(LightData)*m_lights.size(), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
		m_lightBuffer.init(); m_lightBuffer.allocate(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

		/*DescriptorPool*/ {
			VkDescriptorBufferInfo uniformBufferInfo;
			uniformBufferInfo.buffer = m_uniformBuffer;
			uniformBufferInfo.offset = 0;
			uniformBufferInfo.range = m_uniformBuffer.getSize();

			vk::Descriptor uniformBufferDescriptor{};
			uniformBufferDescriptor.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			uniformBufferDescriptor.stages = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
			uniformBufferDescriptor.binding = 0;
			uniformBufferDescriptor.pBufferInfo = &uniformBufferInfo;

			VkDescriptorBufferInfo lightBufferInfo;
			lightBufferInfo.buffer = m_lightBuffer;
			lightBufferInfo.offset = 0;
			lightBufferInfo.range = m_lightBuffer.getSize();

			vk::Descriptor lightBufferDescriptor{};
			lightBufferDescriptor.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
			lightBufferDescriptor.stages = VK_SHADER_STAGE_FRAGMENT_BIT;
			lightBufferDescriptor.binding = 1;
			lightBufferDescriptor.pBufferInfo = &lightBufferInfo;

			m_descriptorPool.addDescriptorSet();
			m_descriptorPool.addDescriptor(uniformBufferDescriptor, 0);
			m_descriptorPool.addDescriptor(lightBufferDescriptor, 0);
			m_descriptorPool.init();
		}

		/*Shader*/
#ifdef _DEBUG
		vk::Shader::compile("Shader/src/", {"PBRShader.vert", "PBRShader.frag"}, {"./"});
#endif

		m_vertexShader.setStage(VK_SHADER_STAGE_VERTEX_BIT);
		m_vertexShader.setPath("PBRShader.vert.spv");

		m_fragmentShader.setStage(VK_SHADER_STAGE_FRAGMENT_BIT);
		m_fragmentShader.setPath("PBRShader.frag.spv");

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

	void PBRenderer::render(Camera* cam) {
		if (glfwGetWindowAttrib(m_window.getGLFWwindow(), GLFW_ICONIFIED)) return;

		m_window.clear();

		for (VisibleActor* actor : m_actors) {
			m_ubo.model = actor->getTransform();
			m_ubo.modelNormal = glm::transpose(glm::inverse(actor->getTransform()));
			m_ubo.view = cam->getView();
			m_ubo.perspective = cam->getPerspective(m_viewport.width / m_viewport.height);
			m_ubo.color = actor->m_color;
			m_ubo.lightCount = m_lights.size();

			void* rawData; m_uniformBuffer.map(&rawData);
			memcpy(rawData, &m_ubo, sizeof(UniformBufferObject));
			m_uniformBuffer.unmap();

			m_lightBuffer.map(&rawData);
			{
				for (uint32_t i = 0; i < m_lights.size(); i++) {
					LightData* lightData = (LightData*)(rawData);
					lightData[i].pos = m_lights[i]->getPos();
					lightData[i].color = m_lights[i]->getColor();
				}

			}
			m_lightBuffer.unmap();

			actor->getModel()->getCommandBuffer(m_window.getCurrentImageIndex())->submit(m_renderComplete);
			vk::waitForFence(m_renderComplete);
		}
	}

	void PBRenderer::addLight(Light* pLight) {
		m_lights.push_back(pLight);
	}
}