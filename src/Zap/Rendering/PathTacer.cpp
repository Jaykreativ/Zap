#include "Zap/Rendering/PathTacer.h"

#include "Zap/Rendering/Renderer.h"
#include "Zap/Scene/Scene.h"
#include "Zap/Scene/Actor.h"

struct UBO {
	glm::mat4 inverseView = glm::mat4(1);
	glm::mat4 inversePerspective = glm::mat4(1);
	uint32_t lightCount = 0;
	uint32_t frameIndex = 0;
};

void updateLightBufferDescriptorSetPT(vk::Registerable* obj, vk::Registerable* dependency, vk::RegisteryFunction func) {
	if (func != vk::eUPDATE)
		return;

	vk::Buffer* pBuffer = (vk::Buffer*)obj;
	vk::DescriptorSet* pDescriptorSet = (vk::DescriptorSet*)dependency;
	auto descriptor = pDescriptorSet->getDescriptor(1);
	descriptor.bufferInfos[0].range = pBuffer->getSize();
	pDescriptorSet->setDescriptor(1, descriptor);

	pDescriptorSet->update();
}

namespace Zap {
	PathTracer::PathTracer(Renderer& renderer, Scene* pScene)
		: m_renderer(renderer), m_pScene(pScene)
	{}

	PathTracer::~PathTracer() {}

	void PathTracer::updateCamera(const Actor camera) {
		ZP_ASSERT(camera.hasCamera(), "ERROR: Actor has no camera component");
		void* rawData; m_UBO.map(&rawData);
		UBO* data = (UBO*)rawData;
		auto oldView = data->inverseView;
		data->inverseView = glm::inverse(camera.cmpCamera_getView());
		if (oldView != data->inverseView)
			m_frameIndex = 0;
		data->inversePerspective = glm::inverse(camera.cmpCamera_getPerspective(m_extent.x / m_extent.y));
		data->lightCount = m_pScene->m_lightComponents.size();
		m_UBO.unmap();
	}

	void PathTracer::resize() {
		uint32_t width = m_pTarget->getExtent().width;
		uint32_t height = m_pTarget->getExtent().height;
		if (width <= 0) width = 1;
		if (height <= 0) height = 1;
		m_extent = { width, height };
		m_storageImage.resize(width, height);

		auto desc = m_rtDescriptorSet.getDescriptor(1);
		vk::DescriptorImageInfo imageInfo{};
		imageInfo.pImage = m_pTarget;
		imageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
		desc.imageInfos = { imageInfo };
		m_rtDescriptorSet.setDescriptor(1, desc);

		desc = m_rtDescriptorSet.getDescriptor(2);
		vk::DescriptorImageInfo storageImageInfo{};
		storageImageInfo.pImage = &m_storageImage;
		storageImageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
		desc.imageInfos = { storageImageInfo };
		m_rtDescriptorSet.setDescriptor(2, desc);

		m_rtDescriptorSet.update();

		m_frameIndex = 0;
	}

	void PathTracer::setRenderTarget(Image* target) {
		m_pTarget = target;
	}

	void PathTracer::setDefaultRenderTarget() {
		m_pTarget = nullptr;
	}

	Image* PathTracer::getRenderTarget() {
		return m_pTarget;
	}

	void PathTracer::onRendererInit() {
		for (uint32_t id : m_pScene->m_meshReferences) {
			auto* base = Base::getBase();
			Mesh* mesh = &base->m_meshes[id];
			if (m_blasMap.count(id)) continue;
			vk::AccelerationStructure& accelerationStructure = m_blasMap[id] = vk::AccelerationStructure();
			accelerationStructure.setType(VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR);
			accelerationStructure.addGeometry(mesh->m_vertexBuffer, sizeof(Vertex), mesh->m_indexBuffer);
			accelerationStructure.init();
		}
		for (auto const& lightPair : m_pScene->m_lightComponents) {
			float aabbMin[3] = { -lightPair.second.radius, -lightPair.second.radius, -lightPair.second.radius };
			float aabbMax[3] = {  lightPair.second.radius,  lightPair.second.radius,  lightPair.second.radius };
			m_lightBlasVector.push_back(vk::AccelerationStructure());
			vk::AccelerationStructure& accelerationStructure = m_lightBlasVector.back();
			accelerationStructure.setType(VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR);
			accelerationStructure.addGeometry(aabbMin, aabbMax);
			accelerationStructure.init();
		}
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
		i = 0;
		for (auto const& lightPair : m_pScene->m_lightComponents) {
			instanceVector.push_back(vk::AccelerationStructureInstance(m_lightBlasVector[i]));
			auto* transform = &glm::transpose(m_pScene->m_transformComponents.at(lightPair.first).transform);
			instanceVector.back().setTransform(*((VkTransformMatrixKHR*)transform));
			instanceVector.back().setCustomIndex(i);
			instanceVector.back().setMask(0x0F);
			i++;
		}
		m_tlas = vk::AccelerationStructure();
		m_tlas.setType(VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR);
		m_tlas.addGeometry(instanceVector);
		m_tlas.init();

		m_storageImage = vk::Image();
		m_storageImage.setWidth(m_extent.x);
		m_storageImage.setHeight(m_extent.y);
		m_storageImage.setAspect(VK_IMAGE_ASPECT_COLOR_BIT);
		m_storageImage.setUsage(VK_IMAGE_USAGE_STORAGE_BIT);
		m_storageImage.setFormat(VK_FORMAT_R32G32B32A32_SFLOAT);
		
		m_storageImage.init();
		m_storageImage.allocate(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
		m_storageImage.initView();

		m_storageImage.changeLayout(VK_IMAGE_LAYOUT_GENERAL, VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT);

		m_UBO = vk::Buffer(sizeof(UBO), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
		m_UBO.init(); m_UBO.allocate(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
		{
			void* rawData; m_UBO.map(&rawData);
			UBO* data = (UBO*)rawData;
			*data = UBO();
			m_UBO.unmap();
		}

#ifdef _DEBUG
		vk::Shader::compile("../Zap/Shader/src/", { "pathTrace.rgen", "pathTrace.rchit", "pathTrace.rmiss", "pathTrace.rint"}, {"./"});
#endif

		m_rgenShader.setPath("pathTrace.rgen.spv");
		m_rchitShader.setPath("pathTrace.rchit.spv");
		m_rmissShader.setPath("pathTrace.rmiss.spv");
		m_rintShader.setPath("pathTrace.rint.spv");

		m_rgenShader.setStage(VK_SHADER_STAGE_RAYGEN_BIT_KHR);
		m_rmissShader.setStage(VK_SHADER_STAGE_MISS_BIT_KHR);
		m_rchitShader.setStage(VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR);
		m_rintShader.setStage(VK_SHADER_STAGE_INTERSECTION_BIT_KHR);

		m_rgenShader.init();
		m_rmissShader.init();
		m_rchitShader.init();
		m_rintShader.init();

		enum {
			eRGEN = 0,
			eRMISS = 1,
			eRCHIT = 2,
			eRINT = 3
		};

		m_rtPipeline.addShader(m_rgenShader.getShaderStage());
		m_rtPipeline.addShader(m_rmissShader.getShaderStage());
		m_rtPipeline.addShader(m_rchitShader.getShaderStage());
		m_rtPipeline.addShader(m_rintShader.getShaderStage());

		VkRayTracingShaderGroupCreateInfoKHR group{ VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR };
		group.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
		group.generalShader = eRGEN;
		group.closestHitShader = VK_SHADER_UNUSED_KHR;
		group.anyHitShader = VK_SHADER_UNUSED_KHR;
		group.intersectionShader = VK_SHADER_UNUSED_KHR;

		m_rtPipeline.addGroup(group);

		group.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
		group.generalShader = eRMISS;

		m_rtPipeline.addGroup(group);

		group.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_PROCEDURAL_HIT_GROUP_KHR;
		group.generalShader = VK_SHADER_UNUSED_KHR;
		group.closestHitShader = eRCHIT;
		group.intersectionShader = eRINT;

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

		vk::DescriptorImageInfo imageInfo{};
		imageInfo.pSampler = nullptr; // TODO add direct write to swapchain
		imageInfo.pImage = m_pTarget;
		imageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;

		m_pTarget->changeLayout(VK_IMAGE_LAYOUT_GENERAL, VK_ACCESS_SHADER_WRITE_BIT | VK_ACCESS_SHADER_READ_BIT);

		descriptor = {};
		descriptor.type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
		descriptor.stages = VK_SHADER_STAGE_RAYGEN_BIT_KHR;
		descriptor.binding = 1;
		descriptor.imageInfos = { imageInfo };

		m_rtDescriptorSet.addDescriptor(descriptor);

		vk::DescriptorImageInfo storageImageInfo{};
		storageImageInfo.pSampler = nullptr;
		storageImageInfo.pImage = &m_storageImage;
		storageImageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;

		descriptor = {};
		descriptor.type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
		descriptor.stages = VK_SHADER_STAGE_RAYGEN_BIT_KHR;
		descriptor.binding = 2;
		descriptor.imageInfos = { storageImageInfo };

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
		lightBufferDescriptor.stages = VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR | VK_SHADER_STAGE_INTERSECTION_BIT_KHR;
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

		Base* base = Base::getBase();
		std::vector<vk::DescriptorImageInfo> textureInfos;
		for (uint32_t i = 0; i < base->m_textures.size(); i++) {
			vk::DescriptorImageInfo textureInfo{};
			textureInfo.pSampler = &base->m_textureSampler;
			textureInfo.pImage = &base->m_textures[i];
			textureInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
			textureInfos.push_back(textureInfo);
		}

		vk::Descriptor texturesDescriptor{};
		texturesDescriptor.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		texturesDescriptor.count = base->m_textures.size();
		texturesDescriptor.stages = VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;
		texturesDescriptor.binding = 3;
		texturesDescriptor.imageInfos = textureInfos;

		m_descriptorSet.addDescriptor(texturesDescriptor);

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

		base->m_registery.connect(&m_pScene->m_lightBuffer, &m_descriptorSet, updateLightBufferDescriptorSetPT);
	}

	void PathTracer::destroy() {
		m_rtPipeline.destroy();
		m_rtDescriptorSet.destroy();
		m_descriptorSet.destroy();
		m_descriptorPool.destroy();
		m_rgenShader.~Shader();
		m_rchitShader.~Shader();
		m_rmissShader.~Shader();
		m_rintShader.~Shader();
		m_UBO.destroy();
		m_storageImage.destroy();
		m_tlas.destroy();
		for (auto& blasPair : m_blasMap) {
			blasPair.second.destroy();
		}
		for (auto& blas : m_lightBlasVector) {
			blas.destroy();
		}
	}

	void PathTracer::beforeRender() {
		if (m_pScene->m_lightBuffer.getSize() != m_oldLightbufferSize) {
			m_oldLightbufferSize = m_pScene->m_lightBuffer.getSize();
			m_renderer.recordCommandBuffers();
		}

		void* rawData; m_UBO.map(&rawData);
		UBO* data = (UBO*)rawData;
		data->frameIndex = m_frameIndex;
		m_UBO.unmap();

		// triangle objects
		if (m_frameIndex > 0) return;
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

		// procedural lights
		i = 0;
		for (auto const& lightPair : m_pScene->m_lightComponents) {
			instanceVector.push_back(vk::AccelerationStructureInstance(m_lightBlasVector[i]));
			auto* transform = &glm::transpose(m_pScene->m_transformComponents.at(lightPair.first).transform);
			instanceVector.back().setTransform(*((VkTransformMatrixKHR*)transform));
			instanceVector.back().setCustomIndex(i);
			instanceVector.back().setMask(0x0F);
			i++;
		}

		m_tlas.updateGeometry(instanceVector);
		m_tlas.update();
	}

	void PathTracer::afterRender() {
		m_frameIndex++;
	}

	void PathTracer::recordCommands(const vk::CommandBuffer* cmd, uint32_t imageIndex) {
		vkCmdBindPipeline(*cmd, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, m_rtPipeline);
		std::vector<VkDescriptorSet> boundSets = {
			m_rtDescriptorSet,
			m_descriptorSet
		};
		vkCmdBindDescriptorSets(*cmd, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, m_rtPipeline.getVkPipelineLayout(), 0, boundSets.size(), boundSets.data(), 0, nullptr);

		vkCmdTraceRaysKHR(*cmd, &m_rtPipeline.getRayGenRegion(), &m_rtPipeline.getMissRegion(), &m_rtPipeline.getHitRegion(), &m_rtPipeline.getCallRegion(), m_extent.x, m_extent.y, 1);
	}

	void PathTracer::onWindowResize(int width, int height) {}
}