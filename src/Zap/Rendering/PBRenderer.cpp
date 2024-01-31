#include "Zap/Rendering/PBRenderer.h"
#include "Zap/Rendering/Renderer.h"
#include "Zap/Scene/Scene.h"
#include "Zap/Scene/Actor.h"
#include "Zap/Scene/Transform.h"
#include "Zap/Scene/Model.h"
#include "Zap/Scene/Light.h"
#include "Zap/Scene/Camera.h"

namespace Zap {
	PBRenderer::PBRenderer(Renderer& renderer, Scene* pScene)
		: m_renderer(renderer), m_pScene(pScene)
	{}

	PBRenderer::PBRenderer(const PBRenderer& pbrenderer)
		: m_renderer(pbrenderer.m_renderer)
	{}

	PBRenderer::~PBRenderer() {}

	void PBRenderer::init() {
		if (m_isInit) return;
		m_isInit = true;

		/*UniformBuffers*/
		m_uniformBuffer = vk::Buffer(sizeof(UniformBufferObject), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
		m_uniformBuffer.init(); m_uniformBuffer.allocate(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

		m_lightBuffer = vk::Buffer(sizeof(LightData) * std::max<size_t>(m_pScene->m_lightComponents.size(), 1), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
		m_lightBuffer.init(); m_lightBuffer.allocate(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

		{
			uint32_t meshCount = 0;
			for (auto const& x : m_pScene->m_modelComponents) meshCount += x.second.m_meshes.size();
			m_perMeshBuffer = vk::Buffer(sizeof(PerMeshData) * meshCount, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
			m_perMeshBuffer.init(); m_perMeshBuffer.allocate(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
		}

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

			m_descriptorSet.addDescriptor(uniformBufferDescriptor);

			VkDescriptorBufferInfo lightBufferInfo;
			lightBufferInfo.buffer = m_lightBuffer;
			lightBufferInfo.offset = 0;
			lightBufferInfo.range = m_lightBuffer.getSize();

			vk::Descriptor lightBufferDescriptor{};
			lightBufferDescriptor.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
			lightBufferDescriptor.stages = VK_SHADER_STAGE_FRAGMENT_BIT;
			lightBufferDescriptor.binding = 1;
			lightBufferDescriptor.pBufferInfo = &lightBufferInfo;

			m_descriptorSet.addDescriptor(lightBufferDescriptor);

			VkDescriptorBufferInfo perMeshBufferInfo;
			perMeshBufferInfo.buffer = m_perMeshBuffer;
			perMeshBufferInfo.offset = 0;
			perMeshBufferInfo.range = m_perMeshBuffer.getSize();

			vk::Descriptor perMeshBufferDescriptor{};
			perMeshBufferDescriptor.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
			perMeshBufferDescriptor.stages = VK_SHADER_STAGE_VERTEX_BIT;
			perMeshBufferDescriptor.binding = 2;
			perMeshBufferDescriptor.pBufferInfo = &perMeshBufferInfo;

			m_descriptorSet.addDescriptor(perMeshBufferDescriptor);

			m_descriptorPool.addDescriptorSet(m_descriptorSet);
			m_descriptorPool.init();

			m_descriptorSet.init();
			m_descriptorSet.allocate();
			m_descriptorSet.update();
		}

		/*Depth Image*/
		m_depthImage = vk::Image();
		m_depthImage.setAspect(VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT);
		m_depthImage.setExtent({ m_renderer.m_window.m_width, m_renderer.m_window.m_height, 1 });//TODO try with viewport scale
		m_depthImage.setFormat(Zap::GlobalSettings::getDepthStencilFormat());
		m_depthImage.setInitialLayout(VK_IMAGE_LAYOUT_UNDEFINED);
		m_depthImage.setType(VK_IMAGE_TYPE_2D);
		m_depthImage.setUsage(VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT);
		m_depthImage.setMemoryProperties(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
		m_depthImage.init();
		m_depthImage.allocate(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
		m_depthImage.initView();

		m_depthImage.changeLayout(VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT);

		/*RenderPass*/
		{
			VkAttachmentDescription colorAttachment;// Color Attachment
			colorAttachment.flags = 0;
			colorAttachment.format = Zap::GlobalSettings::getColorFormat();
			colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
			colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
			colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
			colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			colorAttachment.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;//TODO lookup what this means
			colorAttachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

			m_renderPass.addAttachmentDescription(colorAttachment);

			VkAttachmentReference* pColorAttachmentReference;
			{
				VkAttachmentReference tmp;
				tmp.attachment = 0;
				tmp.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
				pColorAttachmentReference = &tmp;

				m_renderPass.addAttachmentReference(&pColorAttachmentReference);
			}

			VkAttachmentDescription depthStencilAttachment;// Depth Stencil Attachment
			depthStencilAttachment.flags = 0;
			depthStencilAttachment.format = Zap::GlobalSettings::getDepthStencilFormat();
			depthStencilAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
			depthStencilAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
			depthStencilAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
			depthStencilAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			depthStencilAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			depthStencilAttachment.initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
			depthStencilAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

			m_renderPass.addAttachmentDescription(depthStencilAttachment);

			VkAttachmentReference* pDepthStencilAttachmentReference;
			{
				VkAttachmentReference tmp;
				tmp.attachment = 1;
				tmp.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
				pDepthStencilAttachmentReference = &tmp;

				m_renderPass.addAttachmentReference(&pDepthStencilAttachmentReference);
			}

			VkSubpassDescription subpassDescription;
			subpassDescription.flags = 0;
			subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
			subpassDescription.inputAttachmentCount = 0;
			subpassDescription.pInputAttachments = nullptr;
			subpassDescription.colorAttachmentCount = 1;
			subpassDescription.pColorAttachments = pColorAttachmentReference;
			subpassDescription.pResolveAttachments = nullptr;
			subpassDescription.pDepthStencilAttachment = pDepthStencilAttachmentReference;
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

			m_renderPass.init();
		}

		/*Framebuffer*/
		m_framebuffers.resize(m_renderer.m_swapchain.getImageCount());
		for (int i = 0; i < m_renderer.m_swapchain.getImageCount(); i++) {
			m_framebuffers[i].setWidth(m_renderer.m_window.m_width);
			m_framebuffers[i].setHeight(m_renderer.m_window.m_height);
			m_framebuffers[i].addAttachment(m_renderer.m_swapchain.getImageView(i));
			m_framebuffers[i].addAttachment(m_depthImage.getVkImageView());
			m_framebuffers[i].setRenderPass(m_renderPass);
			m_framebuffers[i].init();
		}

		/*Shader*/
#ifdef _DEBUG
		vk::Shader::compile("../Zap/Shader/src/", { "PBRShader.vert", "PBRShader.frag" }, { "./" });
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

		m_pipeline.addDescriptorSetLayout(m_descriptorSet.getVkDescriptorSetLayout());
		for (auto attributeDescription : Vertex::getVertexInputAttributeDescriptions()) {
			m_pipeline.addVertexInputAttrubuteDescription(attributeDescription);
		}
		m_pipeline.addVertexInputBindingDescription(Vertex::getVertexInputBindingDescription());
		m_pipeline.addDynamicState(VK_DYNAMIC_STATE_VIEWPORT);
		m_pipeline.addDynamicState(VK_DYNAMIC_STATE_SCISSOR);
		m_pipeline.addViewport(m_viewport);
		m_pipeline.addScissor(m_scissor);
		m_pipeline.setRenderPass(m_renderPass);
		m_pipeline.enableDepthTest();

		VkPushConstantRange pushConstantRange{};
		pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
		pushConstantRange.offset = 0;
		pushConstantRange.size = sizeof(uint32_t);

		m_pipeline.addPushConstantRange(pushConstantRange);

		m_pipeline.init();
	}

	void PBRenderer::destroy() {
		if (!m_isInit) return;
		m_isInit = false;

		m_pipeline.~Pipeline();
		m_fragmentShader.~Shader();
		m_vertexShader.~Shader();
		for (vk::Framebuffer framebuffer : m_framebuffers) framebuffer.destroy();
		m_renderPass.~RenderPass();
		m_depthImage.destroy();
		m_descriptorSet.destroy();
		m_descriptorPool.destroy();
		m_uniformBuffer.destroy();
		m_lightBuffer.destroy();
		m_perMeshBuffer.destroy();
	}

	void PBRenderer::beforeRender() {}

	void PBRenderer::afterRender() {}

	void PBRenderer::recordCommands(const vk::CommandBuffer* cmd, uint32_t imageIndex) {
		VkRenderPassBeginInfo renderPassBeginInfo;
		renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassBeginInfo.pNext = nullptr;
		renderPassBeginInfo.renderPass = m_renderPass;
		renderPassBeginInfo.framebuffer = m_framebuffers[imageIndex];
		VkRect2D renderArea{};
		int32_t restX = m_renderer.m_window.m_width - (m_scissor.extent.width + m_scissor.offset.x);
		renderArea.offset.x = std::max<int32_t>(0, m_renderer.m_window.m_width - (m_scissor.extent.width + std::max<int32_t>(0, restX)));
		renderArea.extent.width = std::min<int32_t>(m_renderer.m_window.m_width - (m_scissor.offset.x + restX), m_renderer.m_window.m_width);
		int32_t restY = m_renderer.m_window.m_height - (m_scissor.extent.height + m_scissor.offset.y);
		renderArea.offset.y = std::max<int32_t>(0, m_renderer.m_window.m_height - (m_scissor.extent.height + std::max<int32_t>(0, restY)));
		renderArea.extent.height = std::min<int32_t>(m_renderer.m_window.m_height - (m_scissor.offset.y + restY), m_renderer.m_window.m_height);
		renderPassBeginInfo.renderArea = renderArea;
		VkClearValue clearValue = { 0.0f, 0.0f, 0.0f, 1.0f };
		VkClearValue depthClearValue = { 1.0f, 0 };
		std::vector<VkClearValue> clearValues = {
			clearValue,
			depthClearValue
		};
		renderPassBeginInfo.clearValueCount = clearValues.size();
		renderPassBeginInfo.pClearValues = clearValues.data();

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

		uint32_t i = 0;
		for (auto const& modelPair : m_pScene->m_modelComponents) {
			for (uint32_t id : modelPair.second.m_meshes) {
				Mesh* mesh = &Mesh::all[id];

				VkDeviceSize offsets[] = { 0 };
				VkBuffer vertexBuffer = mesh->m_vertexBuffer;
				vkCmdBindVertexBuffers(*cmd, 0, 1, &vertexBuffer, offsets);
				vkCmdBindIndexBuffer(*cmd, mesh->m_indexBuffer, 0, VK_INDEX_TYPE_UINT32);

				VkDescriptorSet boundSets[] = { m_descriptorSet };
				vkCmdBindDescriptorSets(*cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline.getVkPipelineLayout(), 0, 1, boundSets, 0, nullptr);

				vkCmdPushConstants(*cmd, m_pipeline.getVkPipelineLayout(), VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(uint32_t), &i);

				vkCmdDrawIndexed(*cmd, mesh->getIndexbuffer()->getSize() / sizeof(uint32_t), 1, 0, 0, 0);
				i++;
			}
		}

		vkCmdEndRenderPass(*cmd);
	}

	void PBRenderer::onWindowResize(int width, int height) {
		m_depthImage.setWidth(width);
		m_depthImage.setHeight(height);
		m_depthImage.update();
		
		m_depthImage.changeLayout(VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT);

		for (uint32_t i = 0; i < m_renderer.m_swapchain.getImageCount(); i++) {
			m_framebuffers[i].setWidth(width);
			m_framebuffers[i].setHeight(height);
			m_framebuffers[i].delAttachment(0);
			m_framebuffers[i].delAttachment(0);
			m_framebuffers[i].addAttachment(m_renderer.m_swapchain.getImageView(i));
			m_framebuffers[i].addAttachment(m_depthImage.getVkImageView());
			m_framebuffers[i].update();
		}
	}

	void PBRenderer::updateBuffers(Actor camera) {
		m_ubo.view = camera.cmpCamera_getView();
		m_ubo.perspective = camera.cmpCamera_getPerspective(m_viewport.width / m_viewport.height);
		m_ubo.lightCount = m_pScene->m_lightComponents.size();

		void* rawData; m_uniformBuffer.map(&rawData);
		memcpy(rawData, &m_ubo, sizeof(UniformBufferObject));
		m_uniformBuffer.unmap();

		if (m_pScene->m_lightComponents.size() > 0) {
			m_lightBuffer.map(&rawData);
			{
				LightData* lightData = (LightData*)(rawData);
				uint32_t i = 0;
				for (auto const& lightPair : m_pScene->m_lightComponents) {
					lightData[i].pos = m_pScene->m_transformComponents.at(lightPair.first).transform[3];
					lightData[i].color = lightPair.second.color;
					i++;
				}

			}
			m_lightBuffer.unmap();
		}

		m_perMeshBuffer.map(&rawData);
		{
			PerMeshData* perMeshData = (PerMeshData*)(rawData);
			uint32_t i = 0;
			for (auto const& modelPair : m_pScene->m_modelComponents) {
				uint32_t j = 0;
				for (uint32_t id : modelPair.second.m_meshes) {
					perMeshData[i].transform = m_pScene->m_transformComponents.at(modelPair.first).transform;
					perMeshData[i].normalTransform = glm::transpose(glm::inverse(perMeshData[i].transform));
					perMeshData[i].color = glm::vec4(modelPair.second.m_Materials[j].m_AlbedoColor, 0);
					j++; i++;
				}
			}
		}
		m_perMeshBuffer.unmap();
	}

	void PBRenderer::changeScene(Scene* pScene) {

	}

	void PBRenderer::setViewport(uint32_t width, uint32_t height, uint32_t x, uint32_t y) {
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

	void PBRenderer::getViewport(uint32_t& width, uint32_t& height, uint32_t& x, uint32_t& y) {
		width = m_viewport.width;
		height = m_viewport.height;
		x = m_viewport.x;
		y = m_viewport.y;
	}
}