#include "Zap/PBRenderer.h"
#include "Zap/Scene/Actor.h"
#include "Zap/Scene/Transform.h"
#include "Zap/Scene/MeshComponent.h"
#include "Zap/Scene/Light.h"
#include "Zap/Scene/Camera.h"

namespace Zap {
	PBRenderer::PBRenderer(Window& window)
		: Renderer(window) {}
	PBRenderer::~PBRenderer() {
		if (!m_isInit) return;
		m_isInit = false;

		vk::allQueuesWaitIdle();

		vk::destroyFence(m_renderComplete);

		for (uint32_t i = 0; i < m_commandBufferCount; i++) {
			m_commandBuffers[i].free();
		}

		m_pipeline.~Pipeline();
		m_fragmentShader.~Shader();
		m_vertexShader.~Shader();
		m_uniformBuffer.destroy();
		m_lightBuffer.destroy();
		m_perMeshBuffer.destroy();
	}

	void PBRenderer::init() {
		if (m_isInit) return;
		m_isInit = true;

		/*UniformBuffers*/
		m_uniformBuffer = vk::Buffer(sizeof(UniformBufferObject), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
		m_uniformBuffer.init(); m_uniformBuffer.allocate(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

		m_lightBuffer = vk::Buffer(sizeof(LightData)*Light::all.size(), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
		m_lightBuffer.init(); m_lightBuffer.allocate(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

		m_perMeshBuffer = vk::Buffer(sizeof(PerMeshData)*MeshComponent::all.size(), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
		m_perMeshBuffer.init(); m_perMeshBuffer.allocate(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

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

			VkDescriptorBufferInfo perMeshBufferInfo;
			perMeshBufferInfo.buffer = m_perMeshBuffer;
			perMeshBufferInfo.offset = 0;
			perMeshBufferInfo.range = m_perMeshBuffer.getSize();

			vk::Descriptor perMeshBufferDescriptor{};
			perMeshBufferDescriptor.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
			perMeshBufferDescriptor.stages = VK_SHADER_STAGE_VERTEX_BIT;
			perMeshBufferDescriptor.binding = 2;
			perMeshBufferDescriptor.pBufferInfo = &perMeshBufferInfo;

			m_descriptorPool.addDescriptorSet();
			m_descriptorPool.addDescriptor(uniformBufferDescriptor, 0);
			m_descriptorPool.addDescriptor(lightBufferDescriptor, 0);
			m_descriptorPool.addDescriptor(perMeshBufferDescriptor, 0);
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
		m_pipeline.addDynamicState(VK_DYNAMIC_STATE_VIEWPORT);
		m_pipeline.addDynamicState(VK_DYNAMIC_STATE_SCISSOR);
		m_pipeline.addViewport(m_viewport);
		m_pipeline.addScissor(m_scissor);
		m_pipeline.setRenderPass(*m_window.getRenderPass());
		m_pipeline.enableDepthTest();

		VkPushConstantRange pushConstantRange{};
		pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
		pushConstantRange.offset = 0;
		pushConstantRange.size = sizeof(uint32_t);

		m_pipeline.addPushConstantRange(pushConstantRange);
		
		m_pipeline.init();

		m_commandBufferCount = m_window.m_swapchain.getImageCount();
		m_commandBuffers = new vk::CommandBuffer[m_commandBufferCount];
		for (uint32_t i = 0; i < m_commandBufferCount; i++) {
			m_commandBuffers[i].allocate();
		}

		recordCommandBuffers();

		vk::createFence(&m_renderComplete);
	}

	void PBRenderer::recordCommandBuffers() {
		for (uint32_t i = 0; i < m_commandBufferCount; i++) {
			vk::CommandBuffer* cmd = &m_commandBuffers[i];
			cmd->begin(VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT);

			vk::cmdChangeImageLayout(*cmd, m_window.m_swapchain.getImage(m_window.m_currentImageIndex), m_window.m_swapchain.getImageSubresourceRange(),
				VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
				VK_ACCESS_COLOR_ATTACHMENT_READ_BIT, VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT
			);

			VkRenderPassBeginInfo renderPassBeginInfo;
			renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
			renderPassBeginInfo.pNext = nullptr;
			renderPassBeginInfo.renderPass = *m_window.getRenderPass();
			renderPassBeginInfo.framebuffer = *m_window.getFramebuffer(i - m_id * m_window.getSwapchain()->getImageCount());
			VkRect2D renderArea{};
			int32_t restX = m_window.getWidth() - (m_scissor.extent.width + m_scissor.offset.x);
			renderArea.offset.x = std::max<int32_t>(0, m_window.getWidth() - (m_scissor.extent.width + std::max<int32_t>(0, restX)));
			renderArea.extent.width = std::min<int32_t>(m_window.getWidth() - (m_scissor.offset.x + restX), m_window.getWidth());
			int32_t restY = m_window.getHeight() - (m_scissor.extent.height + m_scissor.offset.y);
			renderArea.offset.y = std::max<int32_t>(0, m_window.getHeight() - (m_scissor.extent.height + std::max<int32_t>(0, restY)));
			renderArea.extent.height = std::min<int32_t>(m_window.getHeight() - (m_scissor.offset.y + restY), m_window.getHeight());
			renderPassBeginInfo.renderArea = renderArea;
			renderPassBeginInfo.clearValueCount = 0;
			renderPassBeginInfo.pClearValues = nullptr;

			vkCmdBeginRenderPass(*cmd, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

			vkCmdBindPipeline(*cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline);

			VkViewport renderAreaViewport{};
			renderAreaViewport.x = renderArea.offset.x;
			renderAreaViewport.y = renderArea.offset.y;
			renderAreaViewport.width = renderArea.extent.width;
			renderAreaViewport.height = renderArea.extent.height;
			renderAreaViewport.minDepth = 0;
			renderAreaViewport.maxDepth = 1;

			vkCmdSetViewport(*cmd, 0, 1, &renderAreaViewport);
			vkCmdSetScissor(*cmd, 0, 1, &renderArea);

			for (MeshComponent& meshCmp : MeshComponent::all) {
				Mesh* mesh = &Mesh::all[meshCmp.m_mesh];

				VkDeviceSize offsets[] = { 0 };
				VkBuffer vertexBuffer = mesh->m_vertexBuffer;
				vkCmdBindVertexBuffers(*cmd, 0, 1, &vertexBuffer, offsets);
				vkCmdBindIndexBuffer(*cmd, mesh->m_indexBuffer, 0, VK_INDEX_TYPE_UINT32);

				VkDescriptorSet descriptorSets[] = { m_descriptorPool.getVkDescriptorSet(0) };
				vkCmdBindDescriptorSets(*cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline.getVkPipelineLayout(), 0, 1, descriptorSets, 0, nullptr);

				vkCmdPushConstants(*cmd, m_pipeline.getVkPipelineLayout(), VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(uint32_t), &meshCmp.m_id);

				vkCmdDrawIndexed(*cmd, mesh->getIndexbuffer()->getSize() / sizeof(uint32_t), 1, 0, 0, 0);
			}


			vkCmdEndRenderPass(*cmd);

			cmd->end();
		}
	}

	void PBRenderer::render(uint32_t cam) {
		if (glfwGetWindowAttrib(m_window.getGLFWwindow(), GLFW_ICONIFIED)) return;

		m_ubo.view = Camera::all[cam].getView();
		m_ubo.perspective = Camera::all[cam].getPerspective(m_viewport.width / m_viewport.height);
		m_ubo.lightCount = Light::all.size();

		void* rawData; m_uniformBuffer.map(&rawData);
		memcpy(rawData, &m_ubo, sizeof(UniformBufferObject));
		m_uniformBuffer.unmap();

		m_lightBuffer.map(&rawData);
		{
			LightData* lightData = (LightData*)(rawData);
			for (uint32_t i = 0; i < Light::all.size(); i++) {
				lightData[i].pos = Light::all[i].m_pActor->getTransformComponent()->getPos();
				lightData[i].color = Light::all[i].getColor();
			}

		}
		m_lightBuffer.unmap();

		m_perMeshBuffer.map(&rawData);
		{
			PerMeshData* perMeshData = (PerMeshData*)(rawData);
			for (uint32_t i = 0; i < MeshComponent::all.size(); i++) {
				perMeshData[i].transform = MeshComponent::all[i].m_pActor->getTransform();
				perMeshData[i].normalTransform = glm::transpose(glm::inverse(perMeshData[i].transform));
				perMeshData[i].color = glm::vec4(MeshComponent::all[i].m_material.m_AlbedoColor, 0);
			}
		}
		m_perMeshBuffer.unmap();

		m_commandBuffers[m_window.m_currentImageIndex].submit(m_renderComplete);
		vk::waitForFence(m_renderComplete);
	}
}