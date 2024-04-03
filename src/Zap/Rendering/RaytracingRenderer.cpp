#include "Zap/Rendering/RaytracingRenderer.h"
#include "Zap/Rendering/Renderer.h"
#include "Zap/Scene/Scene.h"
#include "Zap/Scene/Actor.h"
#include "Zap/Scene/Camera.h"
#include "Zap/Scene/Model.h"
#include "Zap/Scene/Mesh.h"


struct UBO {
	glm::mat4 inverseView = glm::mat4(1);
	glm::mat4 inversePerspective = glm::mat4(1);
	alignas(16) glm::vec3 camPos;
	uint32_t lightCount = 0;
};

namespace Zap {
	RaytracingRenderer::RaytracingRenderer(Renderer& renderer, Scene* pScene)
		: m_renderer(renderer), m_pScene(pScene)
	{}

	RaytracingRenderer::~RaytracingRenderer() {}

	void RaytracingRenderer::onRendererInit() {
		uint32_t i = 0;
		for (uint32_t id : m_pScene->m_meshReferences) {
			auto* base = Base::getBase();
			Mesh* mesh = &base->m_meshes[id];
			if (m_blasMap.count(id)) continue;
			vk::AccelerationStructure& accelerationStructure = m_blasMap[id] = vk::AccelerationStructure();
			accelerationStructure.setType(VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR);
			accelerationStructure.addGeometry(mesh->m_vertexBuffer, sizeof(Vertex), mesh->m_indexBuffer);
			accelerationStructure.init();
			i++;
		}
		std::vector<vk::AccelerationStructureInstance> instanceVector;
		i = 0;
		for (auto const& modelPair : m_pScene->m_modelComponents) {
			glm::mat4* transform = &glm::transpose(m_pScene->m_transformComponents.at(modelPair.first).transform);
			for (uint32_t id : modelPair.second.meshes) {
				instanceVector.push_back(vk::AccelerationStructureInstance(m_blasMap.at(id)));
				instanceVector.back().setTransform(*((VkTransformMatrixKHR*)transform));
				instanceVector.back().setCustomIndex(i);
				i++;
			}
		}
		m_tlas = vk::AccelerationStructure();
		m_tlas.setType(VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR);
		m_tlas.addGeometry(instanceVector);
		m_tlas.init();
		
		m_UBO = vk::Buffer(sizeof(UBO), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
		m_UBO.init(); m_UBO.allocate(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
		{
			void* rawData; m_UBO.map(&rawData);
			UBO* data = (UBO*)rawData;
			*data = UBO();
			m_UBO.unmap();
		}

#ifdef _DEBUG
		vk::Shader::compile("../Zap/Shader/src/", { "raytrace.rgen", "raytrace.rchit", "raytrace.rmiss", "raytraceShadow.rmiss"}, {"./"});
#endif

		m_rgenShader.setPath("raytrace.rgen.spv");
		m_rchitShader.setPath("raytrace.rchit.spv");
		m_rmissShader.setPath("raytrace.rmiss.spv");
		m_rsmissShader.setPath("raytraceShadow.rmiss.spv");

		m_rgenShader.setStage(VK_SHADER_STAGE_RAYGEN_BIT_KHR);
		m_rchitShader.setStage(VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR);
		m_rmissShader.setStage(VK_SHADER_STAGE_MISS_BIT_KHR);
		m_rsmissShader.setStage(VK_SHADER_STAGE_MISS_BIT_KHR);

		m_rgenShader.init();
		m_rchitShader.init();
		m_rmissShader.init();
		m_rsmissShader.init();

		m_rtPipeline.addShader(m_rgenShader.getShaderStage());
		m_rtPipeline.addShader(m_rchitShader.getShaderStage());
		m_rtPipeline.addShader(m_rmissShader.getShaderStage());
		m_rtPipeline.addShader(m_rsmissShader.getShaderStage());

		VkRayTracingShaderGroupCreateInfoKHR group{ VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR };
		group.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
		group.generalShader = 0;
		group.closestHitShader = VK_SHADER_UNUSED_KHR;
		group.anyHitShader = VK_SHADER_UNUSED_KHR;
		group.intersectionShader = VK_SHADER_UNUSED_KHR;

		m_rtPipeline.addGroup(group);

		group.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
		group.generalShader = 2;

		m_rtPipeline.addGroup(group);

		group.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
		group.generalShader = 3;

		m_rtPipeline.addGroup(group);

		group.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_KHR;
		group.generalShader = VK_SHADER_UNUSED_KHR;
		group.closestHitShader = 1;

		m_rtPipeline.addGroup(group);

		VkWriteDescriptorSetAccelerationStructureKHR accelerationStructureDescriptor{};
		accelerationStructureDescriptor.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET_ACCELERATION_STRUCTURE_KHR;
		accelerationStructureDescriptor.accelerationStructureCount = 1;
		VkAccelerationStructureKHR accelerationStructure = m_tlas;
		accelerationStructureDescriptor.pAccelerationStructures = &accelerationStructure;

		vk::Descriptor descriptor{};
		descriptor.pNext = &accelerationStructureDescriptor;
		descriptor.type = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR;
		descriptor.stages = VK_SHADER_STAGE_RAYGEN_BIT_KHR | VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;
		descriptor.binding = 0;

		m_rtDescriptorSet.addDescriptor(descriptor);

		VkDescriptorImageInfo imageInfo{};
		imageInfo.sampler = VK_NULL_HANDLE;// TODO add direct write to swapchain
		imageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
		imageInfo.imageView = m_pTarget->getVkImageView();

		m_pTarget->changeLayout(VK_IMAGE_LAYOUT_GENERAL, VK_ACCESS_SHADER_WRITE_BIT | VK_ACCESS_SHADER_READ_BIT);

		descriptor = {};
		descriptor.type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
		descriptor.stages = VK_SHADER_STAGE_RAYGEN_BIT_KHR;
		descriptor.binding = 1;
		descriptor.pImageInfo = &imageInfo;

		m_rtDescriptorSet.addDescriptor(descriptor);

		VkDescriptorBufferInfo camUBOInfo{};
		camUBOInfo.offset = 0;
		camUBOInfo.range = m_UBO.getSize();
		camUBOInfo.buffer = m_UBO;

		vk::Descriptor camUBODescriptor{};
		camUBODescriptor.binding = 0;
		camUBODescriptor.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		camUBODescriptor.stages = VK_SHADER_STAGE_RAYGEN_BIT_KHR | VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;
		camUBODescriptor.pBufferInfo = &camUBOInfo;

		m_descriptorSet.addDescriptor(camUBODescriptor);

		VkDescriptorBufferInfo lightBufferInfo{};
		lightBufferInfo.offset = 0;
		lightBufferInfo.range = m_pScene->m_lightBuffer.getSize();
		lightBufferInfo.buffer = m_pScene->m_lightBuffer;

		vk::Descriptor lightBufferDescriptor{};
		lightBufferDescriptor.binding = 1;
		lightBufferDescriptor.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		lightBufferDescriptor.stages = VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;
		lightBufferDescriptor.pBufferInfo = &lightBufferInfo;

		m_descriptorSet.addDescriptor(lightBufferDescriptor);

		VkDescriptorBufferInfo perMeshInstanceBufferInfo{};
		perMeshInstanceBufferInfo.offset = 0;
		perMeshInstanceBufferInfo.range = m_pScene->m_perMeshInstanceBuffer.getSize();
		perMeshInstanceBufferInfo.buffer = m_pScene->m_perMeshInstanceBuffer;

		vk::Descriptor perMeshInstanceBufferDescriptor{};
		perMeshInstanceBufferDescriptor.binding = 2;
		perMeshInstanceBufferDescriptor.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		perMeshInstanceBufferDescriptor.stages = VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;
		perMeshInstanceBufferDescriptor.pBufferInfo = &perMeshInstanceBufferInfo;

		m_descriptorSet.addDescriptor(perMeshInstanceBufferDescriptor);

		Base* base = Base::getBase();
		VkDescriptorImageInfo* imageInfos = new VkDescriptorImageInfo[base->m_textures.size()];
		for (uint32_t i = 0; i < base->m_textures.size(); i++) {
			imageInfos[i].sampler = base->m_textureSampler;
			imageInfos[i].imageView = base->m_textures[i].getVkImageView();
			imageInfos[i].imageLayout = VK_IMAGE_LAYOUT_GENERAL;
		}

		vk::Descriptor albedoMapsDescriptor{};
		albedoMapsDescriptor.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		albedoMapsDescriptor.count = base->m_textures.size();
		albedoMapsDescriptor.stages = VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;
		albedoMapsDescriptor.binding = 3;
		albedoMapsDescriptor.pImageInfo = imageInfos;

		m_descriptorSet.addDescriptor(albedoMapsDescriptor);

		m_descriptorPool.addDescriptorSet(m_rtDescriptorSet);
		m_descriptorPool.addDescriptorSet(m_descriptorSet);
		m_descriptorPool.init();

		m_rtDescriptorSet.init();
		m_rtDescriptorSet.allocate();
		m_rtDescriptorSet.update();

		m_descriptorSet.init();
		m_descriptorSet.allocate();
		m_descriptorSet.update();

		m_rtPipeline.addDescriptorSetLayout(m_rtDescriptorSet.getVkDescriptorSetLayout());
		m_rtPipeline.addDescriptorSetLayout(m_descriptorSet.getVkDescriptorSetLayout());
		m_rtPipeline.init(); m_rtPipeline.initShaderBindingTable();
	}

	void RaytracingRenderer::destroy() {
		m_rtPipeline.destroy();
		m_rtDescriptorSet.destroy();
		m_descriptorSet.destroy();
		m_descriptorPool.destroy();
		m_rgenShader.~Shader();
		m_rchitShader.~Shader();
		m_rmissShader.~Shader();
		m_rsmissShader.~Shader();
		m_UBO.destroy();
		m_tlas.destroy();
		for (auto& blasPair : m_blasMap) {
			blasPair.second.destroy();
		}
	}

	void RaytracingRenderer::beforeRender() {
		std::vector<vk::AccelerationStructureInstance> instanceVector;
		uint32_t i = 0;
		for (auto const& modelPair : m_pScene->m_modelComponents) {
			glm::mat4* transform = &glm::transpose(m_pScene->m_transformComponents.at(modelPair.first).transform);
			for (uint32_t id : modelPair.second.meshes) {
				instanceVector.push_back(vk::AccelerationStructureInstance(m_blasMap.at(id)));
				instanceVector.back().setTransform(*((VkTransformMatrixKHR*)transform));
				instanceVector.back().setCustomIndex(i);
				i++;
			}
		}

		m_tlas.updateGeometry(instanceVector);
		m_tlas.update();
	}

	void RaytracingRenderer::afterRender() {}

	void RaytracingRenderer::recordCommands(const vk::CommandBuffer* cmd, uint32_t imageIndex) {
		vkCmdBindPipeline(*cmd, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, m_rtPipeline);
		std::vector<VkDescriptorSet> boundSets = { 
			m_rtDescriptorSet,
			m_descriptorSet
		};
		vkCmdBindDescriptorSets(*cmd, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, m_rtPipeline.getVkPipelineLayout(), 0, boundSets.size(), boundSets.data(), 0, nullptr);

		vkCmdTraceRaysKHR(*cmd, &m_rtPipeline.getRayGenRegion(), &m_rtPipeline.getMissRegion(), &m_rtPipeline.getHitRegion(), &m_rtPipeline.getCallRegion(), m_extent.x, m_extent.y, 1);
	}

	void RaytracingRenderer::onWindowResize(int width, int height) {}

	void RaytracingRenderer::updateCamera(const Actor camera) {
		ZP_ASSERT(camera.hasCamera(), "ERROR: Actor has no camera component");
		void* rawData; m_UBO.map(&rawData);
		UBO* data = (UBO*)rawData;
		data->inverseView = glm::inverse(camera.cmpCamera_getView());
		data->inversePerspective = glm::inverse(camera.cmpCamera_getPerspective(m_extent.x/m_extent.y));
		data->camPos = camera.cmpTransform_getPos();
		data->lightCount = m_pScene->m_lightComponents.size();
		m_UBO.unmap();
	}

	void RaytracingRenderer::resize() {
		uint32_t width = m_pTarget->getExtent().width;
		uint32_t height = m_pTarget->getExtent().height;
		if (width <= 0) width = 1;
		if (height <= 0) height = 1;
		m_extent = {width, height};

		auto desc = m_rtDescriptorSet.getDescriptor(1);
		VkDescriptorImageInfo imageInfo{};
		imageInfo.imageView = m_pTarget->getVkImageView();
		imageInfo.imageLayout = m_pTarget->getLayout();
		desc.pImageInfo = &imageInfo;
		m_rtDescriptorSet.setDescriptor(1, desc);
		m_rtDescriptorSet.updateDescriptor(1);
	}

	void RaytracingRenderer::setRenderTarget(Image* target) {
		m_pTarget = target;
	}

	void RaytracingRenderer::setDefaultRenderTarget() {
		m_pTarget = nullptr;
	}

	Image* RaytracingRenderer::getRenderTarget() {
		return m_pTarget;
	}
}