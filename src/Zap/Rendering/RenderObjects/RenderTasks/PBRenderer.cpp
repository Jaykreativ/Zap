#include "Zap/Rendering/RenderObjects/RenderTasks/PBRenderer.h"
#include "Zap/Rendering/Renderer.h"
#define STB_IMAGE_IMPLEMENTATION
#include "Zap/Rendering/stb_image.h"
#include "Zap/Zap.h"
#include "Zap/Scene/Scene.h"
#include "Zap/Scene/Actor.h"
#include "Zap/Scene/Transform.h"
#include "Zap/Scene/Model.h"
#include "Zap/Scene/Light.h"
#include "Zap/Scene/Camera.h"

#include <array>

void updateLightBufferDescriptorSetPBR(vk::Registerable* obj, vk::Registerable* dependency, vk::RegisteryFunction func) {
	if (func != vk::eUPDATE)
		return;

	vk::Buffer* pBuffer = (vk::Buffer*)obj;
	vk::DescriptorSet* pDescriptorSet = (vk::DescriptorSet*)dependency;
	auto descriptor = pDescriptorSet->getDescriptor(1);
	descriptor.bufferInfos[0].pBuffer = pBuffer;
	descriptor.bufferInfos[0].offset = 0;
	descriptor.bufferInfos[0].range = pBuffer->getSize();
	pDescriptorSet->setDescriptor(1, descriptor);
	
	pDescriptorSet->update();
}

void updatePerMeshBufferDescriptorSetPBR(vk::Registerable* obj, vk::Registerable* dependency, vk::RegisteryFunction func) {
	if (func != vk::eUPDATE)
		return;

	vk::Buffer* pBuffer = (vk::Buffer*)obj;
	vk::DescriptorSet* pDescriptorSet = (vk::DescriptorSet*)dependency;
	auto descriptor = pDescriptorSet->getDescriptor(2);
	descriptor.bufferInfos[0].pBuffer = pBuffer;
	descriptor.bufferInfos[0].offset = 0;
	descriptor.bufferInfos[0].range = pBuffer->getSize();
	pDescriptorSet->setDescriptor(2, descriptor);

	pDescriptorSet->update();
}

namespace Zap {
	PBRenderer::PBRenderer(RenderTargetHandle<> target, Scene* pScene)
		: RenderTask(pScene), m_target(target), m_pScene(pScene)
	{}

	PBRenderer::PBRenderer(const PBRenderer& pbrenderer)
		: m_pScene(pbrenderer.m_pScene)
	{}

	PBRenderer::~PBRenderer() {}

	TaskLayoutTransitions PBRenderer::getLayoutTransitions() {
		TaskLayoutTransitions layouts;
		layouts.addLayout(m_target, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL); // define target layout at time of rendering
		return layouts;
	}

	void PBRenderer::init(const LayoutTransitionHelper& layoutTransitionHelper) {
		/* UniformBuffer */
		m_uniformBuffer = vk::Buffer(sizeof(UniformBufferObject), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
		m_uniformBuffer.init(); m_uniformBuffer.allocate(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

		/* DescriptorSet */
		m_descriptorSet = m_pRenderer->createDescriptorSet<GenericDescriptorSet>(3); // create the main descriptor set with room for 3 descriptor bindings

		// Describe bindings for the descriptorSet layout
		DescriptorSetBinding uniformBufferBinding( // binding #0
			VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, // Type
			VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT // Stages the uniform buffer is used in
		);
		VkDescriptorBufferInfo uniformBufferInfo{ m_uniformBuffer, 0, m_uniformBuffer.getSize() }; // describe the actual buffer object using: handle, offset, size

		DescriptorSetBinding lightBufferBinding( // binding #1
			VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
			VK_SHADER_STAGE_FRAGMENT_BIT
		);
		VkDescriptorBufferInfo lightBufferInfo{ *getSceneLightBuffer(), 0, getSceneLightBuffer()->getSize()};

		DescriptorSetBinding perMeshBufferBinding( // binding #2
			VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
			VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT
		);
		VkDescriptorBufferInfo perMeshBufferInfo{ *getScenePerMeshInstanceBuffer(), 0, getScenePerMeshInstanceBuffer()->getSize()};

		m_descriptorSet->addBinding(uniformBufferBinding);
		m_descriptorSet->addBinding(lightBufferBinding);
		m_descriptorSet->addBinding(perMeshBufferBinding);
		m_descriptorSet->createLayout(); // create the layout using the previously added bindings
		m_descriptorSet->allocate(); // allocates the descriptorSet in this renderers pool

		{ // write references to the buffers to the descriptorSet
			std::array<VkWriteDescriptorSet, 3> writes = {
				m_descriptorSet->writeBuffer(uniformBufferInfo, 0),
				m_descriptorSet->writeBuffer(lightBufferInfo,   1),
				m_descriptorSet->writeBuffer(perMeshBufferInfo, 2)
			};
			m_descriptorSet->write(writes.size(), writes.data());
		}

		/* TextureSet */
		m_textureSet = m_pRenderer->createDescriptorSet<GenericDescriptorSet>();

		Base* base = Base::getBase();// TODO add default texture
		auto* textureMap = RenderTask::getTextureDataMap();
		DescriptorSetBinding texturesBinding(
			VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
			VK_SHADER_STAGE_FRAGMENT_BIT,
			textureMap->size()
		);
		std::vector<VkDescriptorImageInfo> textureImageInfos(textureMap->size());
		for (auto& texturePair : *textureMap) {
			uint32_t i = RenderTask::getTextureIndex(texturePair.first);
			textureImageInfos[i] = { base->m_textureSampler, texturePair.second.image.getVkImageView(), VK_IMAGE_LAYOUT_GENERAL };
		}

		m_textureSet->addBinding(texturesBinding);
		m_textureSet->createLayout();
		m_textureSet->allocate();

		{
			auto write = m_textureSet->writeImage(textureImageInfos.data(), textureImageInfos.size(), 0);
			m_textureSet->write(1, &write);
		}

		//base->m_registery.connect(&m_pScene->m_lightBuffer, &m_descriptorSet, updateLightBufferDescriptorSetPBR); TODO update descriptorSets using the renderers event system
		//base->m_registery.connect(&m_pScene->m_perMeshInstanceBuffer, &m_descriptorSet, updatePerMeshBufferDescriptorSetPBR);

		/*Depth Image*/
		m_depthTarget = m_pRenderer->createRenderTarget<RenderTargetImage>();
		m_depthTarget->setAspect(VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT);
		m_depthTarget->setFormat(Zap::GlobalSettings::getDepthStencilFormat());
		m_depthTarget->setUsage(VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT);
		m_depthTarget->init(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

		m_depthTarget->getImage().changeLayout(VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT);

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
			colorAttachment.initialLayout = layoutTransitionHelper.getTransition(m_target).oldLayout;
			colorAttachment.finalLayout = layoutTransitionHelper.getTransition(m_target).newLayout;

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
		m_framebuffer = m_pRenderer->createFramebuffer(m_renderPass, {m_target, m_depthTarget});

		/*Shader*/ // TODO add Shader system to add custom shader and to compile them in an easy way
		m_vertexShader.setStage(VK_SHADER_STAGE_VERTEX_BIT);
		m_vertexShader.setPath("PBRShader.vert.spv");

		m_fragmentShader.setStage(VK_SHADER_STAGE_FRAGMENT_BIT);
		m_fragmentShader.setPath("PBRShader.frag.spv");

		m_vertexShader.init();
		m_fragmentShader.init();

		/*Pipeline*/
		m_pipeline.addShader(m_vertexShader.getShaderStage());
		m_pipeline.addShader(m_fragmentShader.getShaderStage());

		m_pipeline.addDescriptorSetLayout(m_descriptorSet->getLayout());
		m_pipeline.addDescriptorSetLayout(m_textureSet->getLayout());
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
		pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
		pushConstantRange.offset = 0;
		pushConstantRange.size = sizeof(uint32_t);

		m_pipeline.addPushConstantRange(pushConstantRange);

		m_pipeline.init();

		Base::getBase()->getAssetHandler()->getTextureLoadEventHandler()->addCallback(textureLoadCallback, this);
	}

	void PBRenderer::destroy() {
		Base::getBase()->getAssetHandler()->getTextureLoadEventHandler()->removeCallback(textureLoadCallback, this);
		m_pipeline.destroy();
		m_fragmentShader.destroy();
		m_vertexShader.destroy();
		m_pRenderer->destroyFramebuffer(m_framebuffer);
		m_renderPass.destroy();
		m_pRenderer->destroyRenderTarget(m_depthTarget);
		m_descriptorSet->destroy();
		m_textureSet->destroy();
		m_uniformBuffer.destroy();
	}

	void PBRenderer::beforeRender() {
		m_ubo.lightCount = m_pScene->m_lightComponents.size();

		if (m_areTexturesOutdated)
			updateTextureDescriptor();

		void* rawData; m_uniformBuffer.map(&rawData);
		memcpy(rawData, &m_ubo, sizeof(UniformBufferObject));
		m_uniformBuffer.unmap();
	}

	void PBRenderer::addDescriptorPoolSizes(DescriptorPoolSizeList& poolSizes) {
		poolSizes.addSets(2);
		poolSizes.addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1);
		poolSizes.addPoolSize(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 2);
		poolSizes.addPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1);
	}

	void PBRenderer::recordCommands(const vk::CommandBuffer* cmd) {
		VkRenderPassBeginInfo renderPassBeginInfo;
		renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassBeginInfo.pNext = nullptr;
		renderPassBeginInfo.renderPass = m_renderPass;
		renderPassBeginInfo.framebuffer = *m_framebuffer.get();

		VkRect2D renderArea{};
		int32_t restX, restY;
		uint32_t maxWidth, maxHeight;// render area cant be larger then the image to write to
		maxWidth = m_target->getExtent().width;
		maxHeight = m_target->getExtent().height;

		restX = maxWidth - (m_scissor.extent.width + m_scissor.offset.x);
		restY = maxHeight - (m_scissor.extent.height + m_scissor.offset.y);

		renderArea.offset.x = std::max<int32_t>(0, maxWidth - (m_scissor.extent.width + std::max<int32_t>(0, restX)));
		renderArea.offset.y = std::max<int32_t>(0, maxHeight - (m_scissor.extent.height + std::max<int32_t>(0, restY)));

		renderArea.extent.width = std::min<int32_t>(maxWidth - (m_scissor.offset.x + restX), maxWidth);
		renderArea.extent.height = std::min<int32_t>(maxHeight - (m_scissor.offset.y + restY), maxHeight);

		renderPassBeginInfo.renderArea = renderArea;

		VkClearValue clearValue = { clearColor.x, clearColor.y, clearColor.z, clearColor.a };
		VkClearValue depthClearValue = { clearDepthStencil.x, clearDepthStencil.y };
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
			for (Mesh mesh : modelPair.second.meshes) {
				auto* base = Base::getBase();

				VkDeviceSize offsets[] = { 0 };
				const VkBuffer vertexBuffer = *mesh.getVertexBuffer();
				vkCmdBindVertexBuffers(*cmd, 0, 1, &vertexBuffer, offsets);
				vkCmdBindIndexBuffer(*cmd, *mesh.getIndexBuffer(), 0, VK_INDEX_TYPE_UINT32);

				std::array<VkDescriptorSet, 2> boundSets = { m_descriptorSet, m_textureSet };
				vkCmdBindDescriptorSets(*cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline.getVkPipelineLayout(), 0, boundSets.size(), boundSets.data(), 0, nullptr);

				vkCmdPushConstants(*cmd, m_pipeline.getVkPipelineLayout(), VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(uint32_t), &i);

				vkCmdDrawIndexed(*cmd, mesh.getIndexBuffer()->getSize() / sizeof(uint32_t), 1, 0, 0, 0);
				i++;
			}
		}

		vkCmdEndRenderPass(*cmd);
	}

	void PBRenderer::updateCamera(Actor camera) {
		m_ubo.view = camera.cmpCamera_getView();
		m_ubo.perspective = camera.cmpCamera_getPerspective(m_viewport.width / m_viewport.height);
		m_ubo.camPos = camera.cmpTransform_getPos() + glm::vec3(camera.cmpCamera_getOffset()[3]);
	}

	void PBRenderer::changeScene(Scene* pScene) {
		m_pScene = pScene;
		//updateLightBufferDescriptorSetPBR(&m_pScene->m_lightBuffer, &m_descriptorSet, vk::eUPDATE);
		//updatePerMeshBufferDescriptorSetPBR(&m_pScene->m_perMeshInstanceBuffer, &m_descriptorSet, vk::eUPDATE);
		//Base* base = Base::getBase();
		//base->m_registery.connect(&m_pScene->m_lightBuffer, &m_descriptorSet, updateLightBufferDescriptorSetPBR);
		//base->m_registery.connect(&m_pScene->m_perMeshInstanceBuffer, &m_descriptorSet, updatePerMeshBufferDescriptorSetPBR);
	}

	void PBRenderer::setViewport(uint32_t width, uint32_t height, uint32_t x, uint32_t y) {
		width = std::max<uint32_t>(width, x+1);
		height = std::max<uint32_t>(height, y+1);

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

	void PBRenderer::updateTextureDescriptor() {
		Base* base = Base::getBase();
		auto* textureMap = RenderTask::getTextureDataMap();
		std::vector<vk::DescriptorImageInfo> textureImageInfos(textureMap->size());
		for (auto& texturePair : *textureMap) {
			uint32_t i = RenderTask::getTextureIndex(texturePair.first);
			vk::DescriptorImageInfo textureImageInfo{};
			textureImageInfo.pSampler = &base->m_textureSampler;
			textureImageInfo.pImage = &texturePair.second.image;
			textureImageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
			textureImageInfos[i] = textureImageInfo;
		}

		vk::Descriptor texturesDescriptor{};
		texturesDescriptor.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		texturesDescriptor.count = textureImageInfos.size();
		texturesDescriptor.stages = VK_SHADER_STAGE_FRAGMENT_BIT;
		texturesDescriptor.binding = 0;
		texturesDescriptor.imageInfos = textureImageInfos;
		
		uint32_t oldLoadedTextureCount = m_loadedTextureCount;
		m_loadedTextureCount = textureMap->size();

		//m_descriptorPool.addPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, m_loadedTextureCount - oldLoadedTextureCount);
		//m_descriptorPool.update();
		//
		//m_textureSet.setDescriptor(0, texturesDescriptor);
		//m_descriptorSet.setDescriptorPool(&m_descriptorPool);
		//m_textureSet.update();
		//
		//m_descriptorSet.setDescriptorPool(&m_descriptorPool);
		//m_descriptorSet.update();
		//
		//m_pipeline.setDescriptorSetLayout(0, m_descriptorSet.getVkDescriptorSetLayout());
		//m_pipeline.setDescriptorSetLayout(1, m_textureSet.getVkDescriptorSetLayout());
		//m_pipeline.update();

		m_areTexturesOutdated = false;
	}

	void PBRenderer::textureLoadCallback(Zap::TextureLoadEvent& eventParams, void* customParams) {
		Zap::PBRenderer* pObj = reinterpret_cast<Zap::PBRenderer*>(customParams);
		pObj->m_areTexturesOutdated = true;
	}
}