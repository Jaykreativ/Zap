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

void updateLightBufferDescriptorSetRT(vk::Registerable* obj, vk::Registerable* dependency, vk::RegisteryFunction func) {
	if (func != vk::eUPDATE)
		return;

	vk::Buffer* pBuffer = (vk::Buffer*)obj;
	vk::DescriptorSet* pDescriptorSet = (vk::DescriptorSet*)dependency;
	auto descriptor = pDescriptorSet->getDescriptor(1);
	descriptor.bufferInfos[0].range = pBuffer->getSize();
	pDescriptorSet->setDescriptor(1, descriptor);

	pDescriptorSet->update();
}

void updatePerMeshBufferDescriptorSetRT(vk::Registerable* obj, vk::Registerable* dependency, vk::RegisteryFunction func) {
	if (func != vk::eUPDATE)
		return;

	vk::Buffer* pBuffer = (vk::Buffer*)obj;
	vk::DescriptorSet* pDescriptorSet = (vk::DescriptorSet*)dependency;

	uint32_t descriptorIndex = 2;
	auto descriptor = pDescriptorSet->getDescriptor(descriptorIndex);
	descriptor.bufferInfos[0].range = pBuffer->getSize();
	pDescriptorSet->setDescriptor(descriptorIndex, descriptor);

	pDescriptorSet->update();
}

void updateAccelerationStructureDescriptorSetRT(vk::Registerable* obj, vk::Registerable* dependency, vk::RegisteryFunction func) {
	if (func != vk::eUPDATE)
		return;

	vk::AccelerationStructure* pAccel = (vk::AccelerationStructure*)obj;
	vk::DescriptorSet* pDescriptorSet = (vk::DescriptorSet*)dependency;
	auto descriptor = pDescriptorSet->getDescriptor(0);

	VkWriteDescriptorSetAccelerationStructureKHR* accelerationStructureDescriptor = (VkWriteDescriptorSetAccelerationStructureKHR*)descriptor.pNext;
	accelerationStructureDescriptor->pAccelerationStructures = pAccel->getVkAccelerationStructureKHRptr();

	pDescriptorSet->setDescriptor(0, descriptor);
	pDescriptorSet->update();
}

namespace Zap {
	RaytracingRenderer::RaytracingRenderer(Scene* pScene)
		: m_pScene(pScene)
	{
		auto base = Base::getBase();
		auto settings = base->getSettings();
		ZP_ASSERT(settings->enableRaytracing, "Created Raytracer without enabling raytracing");
	}

	RaytracingRenderer::~RaytracingRenderer() {}

	void RaytracingRenderer::init(uint32_t width, uint32_t height, uint32_t imageCount) {
		auto* base = Base::getBase();
		std::vector<vk::AccelerationStructureInstance> instanceVector;
		uint32_t i = 0;
		for (auto const& modelPair : m_pScene->m_modelComponents) {
			glm::mat4* transform = &glm::transpose(m_pScene->m_transformComponents.at(modelPair.first).transform);
			for (Mesh mesh : modelPair.second.meshes) {
				// if mesh has no blas add new one
				if (!m_blasMap.count(mesh.getHandle())) {
					vk::AccelerationStructure& accelerationStructure = m_blasMap[mesh.getHandle()] = vk::AccelerationStructure();
					accelerationStructure.setType(VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR);
					accelerationStructure.init();
					accelerationStructure.addGeometry(*mesh.getVertexBuffer(), sizeof(Vertex), *mesh.getIndexBuffer());
					accelerationStructure.update();
				}

				instanceVector.push_back(vk::AccelerationStructureInstance(m_blasMap.at(mesh.getHandle())));
				instanceVector.back().setTransform(*((VkTransformMatrixKHR*)transform));
				instanceVector.back().setCustomIndex(i);
				i++;
			}
		}
		m_tlas = vk::AccelerationStructure();
		m_tlas.setType(VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR);
		m_tlas.init();

		m_tlas.setGeometry(instanceVector);

		m_UBO = vk::Buffer(sizeof(UBO), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
		m_UBO.init(); m_UBO.allocate(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
		{
			void* rawData; m_UBO.map(&rawData);
			UBO* data = (UBO*)rawData;
			*data = UBO();
			m_UBO.unmap();
		}

#ifdef _DEBUG
		vk::Shader::compile("../Zap/Shader/src/", { "raytrace.rgen", "raytrace.rchit", "raytrace.rmiss", "raytraceShadow.rmiss" }, { "./" });
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

		VkWriteDescriptorSetAccelerationStructureKHR* accelerationStructureDescriptor = new VkWriteDescriptorSetAccelerationStructureKHR{};
		accelerationStructureDescriptor->sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET_ACCELERATION_STRUCTURE_KHR;
		accelerationStructureDescriptor->accelerationStructureCount = 1;
		accelerationStructureDescriptor->pAccelerationStructures = m_tlas.getVkAccelerationStructureKHRptr();

		vk::Descriptor descriptor{};
		descriptor.pNext = accelerationStructureDescriptor;
		descriptor.type = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR;
		descriptor.stages = VK_SHADER_STAGE_RAYGEN_BIT_KHR | VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;
		descriptor.binding = 0;

		m_rtDescriptorSet.addDescriptor(descriptor);

		vk::DescriptorBufferInfo camUBOInfo{};
		camUBOInfo.pBuffer = &m_UBO;
		camUBOInfo.offset = 0;
		camUBOInfo.range = m_UBO.getSize();

		vk::Descriptor camUBODescriptor{};
		camUBODescriptor.binding = 0;
		camUBODescriptor.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		camUBODescriptor.stages = VK_SHADER_STAGE_RAYGEN_BIT_KHR | VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;
		camUBODescriptor.bufferInfos = { camUBOInfo };

		m_descriptorSet.addDescriptor(camUBODescriptor);

		vk::DescriptorBufferInfo lightBufferInfo{};
		lightBufferInfo.pBuffer = &m_pScene->m_lightBuffer;
		lightBufferInfo.offset = 0;
		lightBufferInfo.range = m_pScene->m_lightBuffer.getSize();

		vk::Descriptor lightBufferDescriptor{};
		lightBufferDescriptor.binding = 1;
		lightBufferDescriptor.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		lightBufferDescriptor.stages = VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;
		lightBufferDescriptor.bufferInfos = { lightBufferInfo };

		m_descriptorSet.addDescriptor(lightBufferDescriptor);

		vk::DescriptorBufferInfo perMeshInstanceBufferInfo{};
		perMeshInstanceBufferInfo.pBuffer = &m_pScene->m_perMeshInstanceBuffer;
		perMeshInstanceBufferInfo.offset = 0;
		perMeshInstanceBufferInfo.range = m_pScene->m_perMeshInstanceBuffer.getSize();

		vk::Descriptor perMeshInstanceBufferDescriptor{};
		perMeshInstanceBufferDescriptor.binding = 2;
		perMeshInstanceBufferDescriptor.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		perMeshInstanceBufferDescriptor.stages = VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;
		perMeshInstanceBufferDescriptor.bufferInfos = { perMeshInstanceBufferInfo };

		m_descriptorSet.addDescriptor(perMeshInstanceBufferDescriptor);

		auto* textureMap = RenderTaskTemplate::getTextureDataMap();
		std::vector<vk::DescriptorImageInfo> textureImageInfos(textureMap->size());
		for (auto& texturePair : *textureMap) {
			uint32_t i = RenderTaskTemplate::getTextureIndex(texturePair.first);
			vk::DescriptorImageInfo textureImageInfo{};
			textureImageInfo.pSampler = &base->m_textureSampler;
			textureImageInfo.pImage = &texturePair.second.image;
			textureImageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
			textureImageInfos[i] = textureImageInfo;
		}

		vk::Descriptor texturesDescriptor{};
		texturesDescriptor.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		texturesDescriptor.count = textureImageInfos.size();
		texturesDescriptor.stages = VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;
		texturesDescriptor.binding = 3;
		texturesDescriptor.imageInfos = textureImageInfos;

		m_descriptorSet.addDescriptor(texturesDescriptor);

		m_targetDescriptorSets.resize(imageCount);

		RenderTaskTemplate::initTargetDependencies();

		m_descriptorPool.addDescriptorSet(m_rtDescriptorSet);
		m_descriptorPool.addDescriptorSet(m_descriptorSet);
		for (auto& descriptorSet : m_targetDescriptorSets)
			m_descriptorPool.addDescriptorSet(descriptorSet);
		m_descriptorPool.init();

		m_rtDescriptorSet.init();
		m_rtDescriptorSet.allocate();
		m_rtDescriptorSet.update();

		m_descriptorSet.init();
		m_descriptorSet.allocate();
		m_descriptorSet.update();

		for (auto& descriptorSet : m_targetDescriptorSets) {
			descriptorSet.init();
			descriptorSet.allocate();
			descriptorSet.update();
		}

		m_rtPipeline.addDescriptorSetLayout(m_rtDescriptorSet.getVkDescriptorSetLayout());
		m_rtPipeline.addDescriptorSetLayout(m_descriptorSet.getVkDescriptorSetLayout());
		if (imageCount > 0)
			m_rtPipeline.addDescriptorSetLayout(m_targetDescriptorSets[0].getVkDescriptorSetLayout());
		m_rtPipeline.init(); m_rtPipeline.initShaderBindingTable();

		base->m_registery.connect(&m_pScene->m_lightBuffer, &m_descriptorSet, updateLightBufferDescriptorSetRT);
		base->m_registery.connect(&m_pScene->m_perMeshInstanceBuffer, &m_descriptorSet, updatePerMeshBufferDescriptorSetRT);
		base->m_registery.connect(&m_tlas, &m_rtDescriptorSet, updateAccelerationStructureDescriptorSetRT);
	}

	void RaytracingRenderer::initTargetDependencies(uint32_t width, uint32_t height, uint32_t imageCount, vk::Image* pTarget, uint32_t imageIndex) {
		vk::DescriptorImageInfo imageInfo{};
		imageInfo.pSampler = nullptr;
		imageInfo.pImage = pTarget;
		imageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;

		vk::Descriptor targetDescriptor{};
		targetDescriptor.type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
		targetDescriptor.stages = VK_SHADER_STAGE_RAYGEN_BIT_KHR;
		targetDescriptor.binding = 0;
		targetDescriptor.imageInfos = { imageInfo };

		m_targetDescriptorSets[imageIndex].addDescriptor(targetDescriptor);
	}

	void RaytracingRenderer::resize(uint32_t width, uint32_t height, uint32_t imageCount) {
		if (width <= 0) width = 1;
		if (height <= 0) height = 1;
		m_extent = { width, height };

		RenderTaskTemplate::resizeTargetDependencies();
	}

	void RaytracingRenderer::resizeTargetDependencies(uint32_t width, uint32_t height, uint32_t imageCount, vk::Image* pTarget, uint32_t imageIndex) {
		auto desc = m_targetDescriptorSets[imageIndex].getDescriptor(0);
		vk::DescriptorImageInfo imageInfo{};
		imageInfo.pImage = pTarget;
		imageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
		desc.imageInfos = { imageInfo };
		m_targetDescriptorSets[imageIndex].setDescriptor(0, desc);
		m_targetDescriptorSets[imageIndex].update();
	}

	void RaytracingRenderer::destroy() {
		m_rtPipeline.destroy();
		m_rtDescriptorSet.destroy();
		m_descriptorSet.destroy();
		for (auto& descriptorSet : m_targetDescriptorSets)
			descriptorSet.destroy();
		m_descriptorPool.destroy();
		m_rgenShader.destroy();
		m_rchitShader.destroy();
		m_rmissShader.destroy();
		m_rsmissShader.destroy();
		m_UBO.destroy();
		m_tlas.destroy();
		for (auto& blasPair : m_blasMap) {
			blasPair.second.destroy();
		}
	}

	void RaytracingRenderer::beforeRender(vk::Image* pTarget, uint32_t imageIndex) {
		auto* base = Base::getBase();
		std::vector<vk::AccelerationStructureInstance> instanceVector;

		uint32_t i = 0;
		for (auto const& modelPair : m_pScene->m_modelComponents) {
			for (Mesh mesh : modelPair.second.meshes) {
				auto* base = Base::getBase();
				glm::mat4* transform = &glm::transpose(m_pScene->m_transformComponents.at(modelPair.first).transform * *mesh.getTransform());

				// if mesh has no blas add new one
				if (!m_blasMap.count(mesh.getHandle())) {
					vk::AccelerationStructure& accelerationStructure = m_blasMap[mesh.getHandle()] = vk::AccelerationStructure();
					accelerationStructure.setType(VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR);
					accelerationStructure.init();
					accelerationStructure.addGeometry(*mesh.getVertexBuffer(), sizeof(Vertex), *mesh.getIndexBuffer());
					accelerationStructure.update();
				}

				instanceVector.push_back(vk::AccelerationStructureInstance(m_blasMap.at(mesh.getHandle())));
				instanceVector.back().setTransform(*((VkTransformMatrixKHR*)transform));
				instanceVector.back().setCustomIndex(i);
				instanceVector.back().setMask(0xFF);
				i++;
			}
		}

		m_tlas.setGeometry(instanceVector);
		m_tlas.update();
	}

	void RaytracingRenderer::afterRender(vk::Image* pTarget, uint32_t imageIndex) {}

	void RaytracingRenderer::recordCommands(const vk::CommandBuffer* cmd, vk::Image* pTarget, uint32_t imageIndex) {
		vkCmdBindPipeline(*cmd, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, m_rtPipeline);
		std::vector<VkDescriptorSet> boundSets = {
			m_rtDescriptorSet,
			m_descriptorSet,
			m_targetDescriptorSets[imageIndex]
		};
		vkCmdBindDescriptorSets(*cmd, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, m_rtPipeline.getVkPipelineLayout(), 0, boundSets.size(), boundSets.data(), 0, nullptr);

		vkCmdTraceRaysKHR(*cmd, &m_rtPipeline.getRayGenRegion(), &m_rtPipeline.getMissRegion(), &m_rtPipeline.getHitRegion(), &m_rtPipeline.getCallRegion(), m_extent.x, m_extent.y, 1);
	}

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
}