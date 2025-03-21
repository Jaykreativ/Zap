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

	uint32_t descriptorIndex = 1;
	auto descriptor = pDescriptorSet->getDescriptor(descriptorIndex);
	descriptor.bufferInfos[0].range = pBuffer->getSize();
	pDescriptorSet->setDescriptor(descriptorIndex, descriptor);

	pDescriptorSet->update();
}

void updatePerMeshBufferDescriptorSetPT(vk::Registerable* obj, vk::Registerable* dependency, vk::RegisteryFunction func) {
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

void updateAccelerationStructureDescriptorSetPT(vk::Registerable* obj, vk::Registerable* dependency, vk::RegisteryFunction func) {
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
	PathTracer::PathTracer(Scene* pScene)
		: m_pScene(pScene)
	{
		auto base = Base::getBase();
		auto settings = base->getSettings();
		ZP_ASSERT(settings->enableRaytracing, "Created PathTracer without enabling raytracing");

		m_pScene->getAddLightEventHandler()->addCallback(addLightCallback, this);
		m_pScene->getRemoveLightEventHandler()->addCallback(removeLightCallback, this);
		m_pScene->getAddModelEventHandler()->addCallback(addModelCallback, this);
		m_pScene->getRemoveModelEventHandler()->addCallback(removeModelCallback, this);
	}

	PathTracer::~PathTracer() {
		m_pScene->getAddLightEventHandler()->removeCallback(addLightCallback, this);
		m_pScene->getRemoveLightEventHandler()->removeCallback(removeLightCallback, this);
		m_pScene->getAddModelEventHandler()->removeCallback(addModelCallback, this);
		m_pScene->getRemoveModelEventHandler()->removeCallback(removeModelCallback, this);
	}

	void PathTracer::updateCamera(const Actor camera) {
		ZP_ASSERT(camera.hasCamera(), "ERROR: Actor has no camera component");
		void* rawData; m_UBO.map(&rawData);
		UBO* data = (UBO*)rawData;
		auto oldView = data->inverseView;
		data->inverseView = glm::inverse(camera.cmpCamera_getView());
		if (oldView != data->inverseView)
			resetRender();
		data->inversePerspective = glm::inverse(camera.cmpCamera_getPerspective(m_extent.x / m_extent.y));
		data->lightCount = m_pScene->m_lightComponents.size();
		m_UBO.unmap();
	}

	void PathTracer::resetRender() {
		m_frameIndex = 0;
	}

	void PathTracer::init(uint32_t width, uint32_t height, uint32_t imageCount) {
		auto* base = Base::getBase();

		// create lightBLAS
		for (auto const& lightPair : m_pScene->m_lightComponents) {
			float aabbMin[3] = { -lightPair.second.radius, -lightPair.second.radius, -lightPair.second.radius };
			float aabbMax[3] = { lightPair.second.radius,  lightPair.second.radius,  lightPair.second.radius };
			vk::AccelerationStructure& accelerationStructure = (m_lightBlasMap[lightPair.first] = vk::AccelerationStructure());
			accelerationStructure.setType(VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR);
			accelerationStructure.init();
			accelerationStructure.addGeometry(aabbMin, aabbMax);
			accelerationStructure.update();
		}
		std::vector<vk::AccelerationStructureInstance> instanceVector;

		// create blas instances, if blas is missing create new
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
		i = 0;

		// create lightBlas instances
		for (auto const& lightPair : m_pScene->m_lightComponents) {
			instanceVector.push_back(vk::AccelerationStructureInstance(m_lightBlasMap.at(lightPair.first)));
			auto* transform = &glm::transpose(m_pScene->m_transformComponents.at(lightPair.first).transform);
			instanceVector.back().setTransform(*((VkTransformMatrixKHR*)transform));
			instanceVector.back().setCustomIndex(i);
			instanceVector.back().setMask(0x0F);
			i++;
		}

		// create tlas
		m_tlas = vk::AccelerationStructure();
		m_tlas.setType(VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR);
		m_tlas.init();

		m_tlas.setGeometry(instanceVector);
		// create storage image for samples over time
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

		vk::DescriptorImageInfo storageImageInfo{};
		storageImageInfo.pSampler = nullptr;
		storageImageInfo.pImage = &m_storageImage;
		storageImageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;

		descriptor = {};
		descriptor.type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
		descriptor.stages = VK_SHADER_STAGE_RAYGEN_BIT_KHR;
		descriptor.binding = 1;
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

		m_loadedTextureCount = textureMap->size();

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

		base->m_registery.connect(&m_pScene->m_lightBuffer, &m_descriptorSet, updateLightBufferDescriptorSetPT);
		base->m_registery.connect(&m_pScene->m_perMeshInstanceBuffer, &m_descriptorSet, updatePerMeshBufferDescriptorSetPT);
		base->m_registery.connect(&m_tlas, &m_rtDescriptorSet, updateAccelerationStructureDescriptorSetPT);
	
		Base::getBase()->getAssetHandler()->getTextureLoadEventHandler()->addCallback(textureLoadCallback, this);
	}

	void PathTracer::initTargetDependencies(uint32_t width, uint32_t height, uint32_t imageCount, vk::Image* pTarget, uint32_t imageIndex) {
		vk::DescriptorImageInfo targetImageInfo{};
		targetImageInfo.pSampler = nullptr;
		targetImageInfo.pImage = pTarget;
		targetImageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;

		vk::Descriptor targetDescriptor{};
		targetDescriptor.type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
		targetDescriptor.stages = VK_SHADER_STAGE_RAYGEN_BIT_KHR;
		targetDescriptor.binding = 0;
		targetDescriptor.imageInfos = { targetImageInfo };

		m_targetDescriptorSets[imageIndex].addDescriptor(targetDescriptor);
	}

	void PathTracer::resize(uint32_t width, uint32_t height, uint32_t imageCount) {
		if (width <= 0) width = 1;
		if (height <= 0) height = 1;
		m_extent = { width, height };

		m_storageImage.resize(width, height);
	
		auto desc = m_rtDescriptorSet.getDescriptor(1);
		vk::DescriptorImageInfo storageImageInfo{};
		storageImageInfo.pImage = &m_storageImage;
		storageImageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
		desc.imageInfos = { storageImageInfo };
		m_rtDescriptorSet.setDescriptor(1, desc);

		m_rtDescriptorSet.update();

		RenderTaskTemplate::resizeTargetDependencies();
	
		resetRender();
	}

	void PathTracer::resizeTargetDependencies(uint32_t width, uint32_t height, uint32_t imageCount, vk::Image* pTarget, uint32_t imageIndex) {
		auto desc = m_targetDescriptorSets[imageIndex].getDescriptor(0);
		vk::DescriptorImageInfo imageInfo{};
		imageInfo.pImage = pTarget;
		imageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
		desc.imageInfos = { imageInfo };
		m_targetDescriptorSets[imageIndex].setDescriptor(0, desc);

		m_targetDescriptorSets[imageIndex].update();
	}

	void PathTracer::destroy() {
		m_rtPipeline.destroy();
		m_rtDescriptorSet.destroy();
		m_descriptorSet.destroy();
		for (auto& descriptorSet : m_targetDescriptorSets)
			descriptorSet.destroy();
		m_descriptorPool.destroy();
		m_rgenShader.destroy();
		m_rchitShader.destroy();
		m_rmissShader.destroy();
		m_rintShader.destroy();
		m_UBO.destroy();
		m_storageImage.destroy();
		m_tlas.destroy();
		for (auto& blasPair : m_blasMap) {
			blasPair.second.destroy();
		}
		m_blasMap.clear();
		for (auto& lightBlasPair : m_lightBlasMap) {
			lightBlasPair.second.destroy();
		}
		m_lightBlasMap.clear();
	}

	void PathTracer::beforeRender(vk::Image* pTarget, uint32_t imageIndex) {
		void* rawData; m_UBO.map(&rawData);
		UBO* data = (UBO*)rawData;
		data->frameIndex = m_frameIndex;
		m_UBO.unmap();

		if (m_frameIndex > 0) return;// only update AccelerationStructure when render is being reset

		// triangle objects
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

		// procedural geometry lights
		i = 0;
		for (auto const& lightPair : m_pScene->m_lightComponents) {
			instanceVector.push_back(vk::AccelerationStructureInstance(m_lightBlasMap.at(lightPair.first)));
			auto* transform = &glm::transpose(m_pScene->m_transformComponents.at(lightPair.first).transform);
			instanceVector.back().setTransform(*((VkTransformMatrixKHR*)transform));
			instanceVector.back().setCustomIndex(i);
			instanceVector.back().setMask(0x0F);
			i++;
		}

		if (m_areTexturesOutdated)
			updateTextureDescriptor();

		m_tlas.setGeometry(instanceVector);
		m_tlas.update();
	}

	void PathTracer::afterRender(vk::Image* pTarget, uint32_t imageIndex) {
		m_frameIndex++;
	}

	void PathTracer::recordCommands(const vk::CommandBuffer* cmd, vk::Image* pTarget, uint32_t imageIndex) {
		vkCmdBindPipeline(*cmd, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, m_rtPipeline);
		std::vector<VkDescriptorSet> boundSets = {
			m_rtDescriptorSet,
			m_descriptorSet,
			m_targetDescriptorSets[imageIndex]
		};
		vkCmdBindDescriptorSets(*cmd, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, m_rtPipeline.getVkPipelineLayout(), 0, boundSets.size(), boundSets.data(), 0, nullptr);

		vkCmdTraceRaysKHR(*cmd, &m_rtPipeline.getRayGenRegion(), &m_rtPipeline.getMissRegion(), &m_rtPipeline.getHitRegion(), &m_rtPipeline.getCallRegion(), m_extent.x, m_extent.y, 1);

	}

	void PathTracer::updateTextureDescriptor() {
		Base* base = Base::getBase();// TODO add default texture
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

		uint32_t oldLoadedTextureCount = m_loadedTextureCount;
		m_loadedTextureCount = textureMap->size();

		m_descriptorPool.addPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, m_loadedTextureCount - oldLoadedTextureCount);
		m_descriptorPool.update();

		m_descriptorSet.setDescriptor(3, texturesDescriptor);
		m_descriptorSet.setDescriptorPool(&m_descriptorPool);
		m_descriptorSet.update();

		m_rtDescriptorSet.setDescriptorPool(&m_descriptorPool);
		m_rtDescriptorSet.update();

		for (auto& targetSet : m_targetDescriptorSets) {
			targetSet.setDescriptorPool(&m_descriptorPool);
			targetSet.update();
		}

		m_rtPipeline.setDescriptorSetLayout(0, m_rtDescriptorSet.getVkDescriptorSetLayout());
		m_rtPipeline.setDescriptorSetLayout(1, m_descriptorSet.getVkDescriptorSetLayout());
		m_rtPipeline.setDescriptorSetLayout(2, m_targetDescriptorSets[0].getVkDescriptorSetLayout());
		m_rtPipeline.update();

		m_areTexturesOutdated = false;
	}

	void PathTracer::textureLoadCallback(Zap::TextureLoadEvent& eventParams, void* customParams) {
		Zap::PathTracer* pObj = reinterpret_cast<Zap::PathTracer*>(customParams);
		pObj->m_areTexturesOutdated = true;
	}


	void PathTracer::addLightCallback(AddLightEvent& eventParams, void* customParams) {
		PathTracer* pPathTracer = (PathTracer*)customParams;
		auto radius = eventParams.actor.cmpLight_getRadius();
		float aabbMin[3] = { -radius, -radius, -radius };
		float aabbMax[3] = { radius, radius, radius };
		vk::AccelerationStructure& accelerationStructure = (pPathTracer->m_lightBlasMap[eventParams.actor] = vk::AccelerationStructure());
		accelerationStructure.setType(VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR);
		accelerationStructure.init();
		accelerationStructure.addGeometry(aabbMin, aabbMax);
		accelerationStructure.update();

		pPathTracer->resetRender();
	}

	void PathTracer::removeLightCallback(RemoveLightEvent& eventParams, void* customParams) {
		PathTracer* pPathTracer = (PathTracer*)customParams;
		vk::AccelerationStructure& accelerationStructure = pPathTracer->m_lightBlasMap.at(eventParams.actor);
		accelerationStructure.destroy();
		pPathTracer->m_lightBlasMap.erase(eventParams.actor);

		pPathTracer->resetRender();
	}

	void PathTracer::addModelCallback(AddModelEvent& eventParams, void* customParams){
		PathTracer* pPathTracer = (PathTracer*)customParams;

		pPathTracer->resetRender();
	}

	void PathTracer::removeModelCallback(RemoveModelEvent& eventParams, void* customParams) {
		PathTracer* pPathTracer = (PathTracer*)customParams;

		pPathTracer->resetRender();
	}
}