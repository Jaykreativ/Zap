#include "Zap/Rendering/RenderObjects/RenderTasks/PathTacer.h"

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

namespace Zap {
	PathTracer::PathTracer(Renderer* pRenderer, RenderTargetHandle<> target, Scene* pScene)
		: RenderTask(pRenderer, pScene), m_target(target), m_pScene(pScene),
		EventListener<RenderEvent::Resize>(getEventHandler()),
		EventListener<AssetHandlerEvent::TextureLoad>(Base::getBase()->getAssetHandler()->getEventHandler()),
		EventListener<SceneEvent::AddModel>(pScene->getEventHandler()),
		EventListener<SceneEvent::RemoveModel>(pScene->getEventHandler()),
		EventListener<SceneEvent::AddLight>(pScene->getEventHandler()),
		EventListener<SceneEvent::RemoveLight>(pScene->getEventHandler()),
		EventListener<SceneEvent::UpdateMeshInstanceBuffer>(pScene->getEventHandler()),
		EventListener<SceneEvent::UpdateLightBuffer>(pScene->getEventHandler())
	{
		auto base = Base::getBase();
		auto settings = base->getSettings();
		ZP_ASSERT(settings->enableRaytracing, "Created PathTracer without enabling raytracing");
	}

	PathTracer::~PathTracer() {}

	void PathTracer::updateCamera(const Actor camera) {
		ZP_ASSERT(camera.hasCamera(), "ERROR: Actor has no camera component");
		void* rawData; m_UBO.map(&rawData);
		UBO* data = (UBO*)rawData;
		auto oldView = data->inverseView;
		data->inverseView = glm::inverse(camera.cmpCamera_getView());
		if (oldView != data->inverseView)
			resetRender();
		auto extent = m_target->getExtent();
		data->inversePerspective = glm::inverse(camera.cmpCamera_getPerspective((float)(extent.width) / extent.height));
		data->lightCount = m_pScene->m_lightComponents.size();
		m_UBO.unmap();
	}

	void PathTracer::resetRender() {
		m_frameIndex = 0;
	}

	void PathTracer::addDescriptorPoolSizes(DescriptorPoolSizeList& poolSizes) {
		poolSizes.addSets(5);
		poolSizes.addPoolSize(VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR, 1);
		poolSizes.addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1);
		poolSizes.addPoolSize(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 2);
		poolSizes.addPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000);
		poolSizes.addPoolSize(VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 10);
	}

	TaskLayoutTransitions PathTracer::getLayoutTransitions() {
		TaskLayoutTransitions transitions;
		transitions.addLayout(m_target, VK_IMAGE_LAYOUT_GENERAL);
		return transitions;
	}

	void PathTracer::init(const LayoutTransitionHelper& layoutTransitionHelper) {
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
			for (auto mesh : modelPair.second.meshes) {
				// if mesh has no blas add new one
				if (!m_blasMap.count(mesh)) {
					vk::AccelerationStructure& accelerationStructure = m_blasMap[mesh] = vk::AccelerationStructure();
					accelerationStructure.setType(VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR);
					accelerationStructure.init();
					accelerationStructure.addGeometry(mesh->getVertexBuffer(), sizeof(Vertex), mesh->getIndexBuffer());
					accelerationStructure.update();
				}

				instanceVector.push_back(vk::AccelerationStructureInstance(m_blasMap.at(mesh)));
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
		m_storageTarget = m_pRenderer->createRenderTarget<RenderTargetImage>(
			VK_FORMAT_R32G32B32A32_SFLOAT,
			VK_IMAGE_USAGE_STORAGE_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
		);

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

		/* DescriptorSets */
		m_rtDescriptorSet = m_pRenderer->createDescriptorSet<GenericDescriptorSet>();

		DescriptorSetBinding accelBinding(VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR, VK_SHADER_STAGE_RAYGEN_BIT_KHR | VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR);
		VkWriteDescriptorSetAccelerationStructureKHR accelInfoKHR = VkWriteDescriptorSetAccelerationStructureKHR{ VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET_ACCELERATION_STRUCTURE_KHR };
		accelInfoKHR.accelerationStructureCount = 1;
		accelInfoKHR.pAccelerationStructures = m_tlas.getVkAccelerationStructureKHRptr();

		m_rtDescriptorSet->addBinding(accelBinding);
		m_rtDescriptorSet->createLayout();
		m_rtDescriptorSet->allocate();
		VkWriteDescriptorSet rtWrites[] = {
			m_rtDescriptorSet->writeGeneric(&accelInfoKHR, 1, 0)
		};
		m_rtDescriptorSet->write(1, rtWrites);

		m_descriptorSet = m_pRenderer->createDescriptorSet<GenericDescriptorSet>();

		DescriptorSetBinding camUBOBinding(
			VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 
			VK_SHADER_STAGE_RAYGEN_BIT_KHR | VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR
		);
		VkDescriptorBufferInfo camUBOInfo{};
		camUBOInfo.buffer = m_UBO;
		camUBOInfo.offset = 0;
		camUBOInfo.range = m_UBO.getSize();

		m_descriptorSet->addBinding(camUBOBinding);

		DescriptorSetBinding lightBufferBinding(
			VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 
			VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR | VK_SHADER_STAGE_INTERSECTION_BIT_KHR
		);
		VkDescriptorBufferInfo lightBufferInfo{};
		lightBufferInfo.buffer = m_pScene->m_lightBuffer;
		lightBufferInfo.offset = 0;
		lightBufferInfo.range = m_pScene->m_lightBuffer.getSize();

		m_descriptorSet->addBinding(lightBufferBinding);

		DescriptorSetBinding perMeshInstanceBufferBinding(
			VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 
			VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR
		);
		VkDescriptorBufferInfo perMeshInstanceBufferInfo{};
		perMeshInstanceBufferInfo.buffer = m_pScene->m_perMeshInstanceBuffer;
		perMeshInstanceBufferInfo.offset = 0;
		perMeshInstanceBufferInfo.range = m_pScene->m_perMeshInstanceBuffer.getSize();

		m_descriptorSet->addBinding(perMeshInstanceBufferBinding);

		m_descriptorSet->createLayout();
		m_descriptorSet->allocate();
		std::array<VkWriteDescriptorSet, 3> writes = {
			m_descriptorSet->writeBuffer(camUBOInfo, 0),
			m_descriptorSet->writeBuffer(lightBufferInfo, 1),
			m_descriptorSet->writeBuffer(perMeshInstanceBufferInfo, 2)
		};
		m_descriptorSet->write(writes.size(), writes.data());

		m_textureSet = m_pRenderer->createDescriptorSet<GenericDescriptorSet>();

		auto* textureMap = RenderTask::getTextureDataMap();
		DescriptorSetBinding texturesBinding(
			VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
			VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR,
			textureMap->size()
		);
		std::vector<VkDescriptorImageInfo> textureImageInfos(textureMap->size());
		for (auto& texturePair : *textureMap) {
			uint32_t i = RenderTask::getTextureIndex(texturePair.first);
			textureImageInfos[i] = { base->m_textureSampler, texturePair.second->getImage(), VK_IMAGE_LAYOUT_GENERAL};
		}
		m_textureSet->addBinding(texturesBinding);
		m_textureSet->createLayout();
		m_textureSet->allocate();
		auto textureWrite = m_textureSet->writeImage(textureImageInfos.data(), textureImageInfos.size(), 0);
		m_textureSet->write(1, &textureWrite);

		m_loadedTextureCount = textureMap->size();

		m_targetDescriptorSet = m_pRenderer->createDescriptorSet<RenderTargetDescriptorSet>(m_target, VK_SHADER_STAGE_RAYGEN_BIT_KHR);
		m_targetDescriptorSet->write();
		m_targetFinalLayout = layoutTransitionHelper.getTransition(m_target).newLayout;

		m_storageDescriptorSet = m_pRenderer->createDescriptorSet<RenderTargetDescriptorSet>(m_storageTarget, VK_SHADER_STAGE_RAYGEN_BIT_KHR);
		m_storageDescriptorSet->write();

		m_rtPipeline.addDescriptorSetLayout(m_rtDescriptorSet->getLayout());
		m_rtPipeline.addDescriptorSetLayout(m_descriptorSet->getLayout());
		m_rtPipeline.addDescriptorSetLayout(m_textureSet->getLayout());
		m_rtPipeline.addDescriptorSetLayout(m_targetDescriptorSet->getLayout());
		m_rtPipeline.addDescriptorSetLayout(m_storageDescriptorSet->getLayout());
		m_rtPipeline.init(); m_rtPipeline.initShaderBindingTable();
	}

	void PathTracer::destroy() {
		m_rtPipeline.destroy();
		m_pRenderer->destroyDescriptorSet(m_rtDescriptorSet);
		m_pRenderer->destroyDescriptorSet(m_descriptorSet);
		m_pRenderer->destroyDescriptorSet(m_targetDescriptorSet);
		m_pRenderer->destroyDescriptorSet(m_storageDescriptorSet);
		m_rgenShader.destroy();
		m_rchitShader.destroy();
		m_rmissShader.destroy();
		m_rintShader.destroy();
		m_UBO.destroy();
		m_pRenderer->destroyRenderTarget(m_storageTarget);
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

	void PathTracer::beforeRender() {
		void* rawData; m_UBO.map(&rawData);
		UBO* data = (UBO*)rawData;
		data->frameIndex = m_frameIndex;
		m_UBO.unmap();

		if (m_frameIndex > 0) return;// only update AccelerationStructure when render is being reset

		// triangle objects
		std::vector<vk::AccelerationStructureInstance> instanceVector;
		uint32_t i = 0;
		for (auto const& modelPair : m_pScene->m_modelComponents) {
			for (auto mesh : modelPair.second.meshes) {
				auto* base = Base::getBase();
				glm::mat4* transform = &glm::transpose(m_pScene->m_transformComponents.at(modelPair.first).transform * mesh->getTransform());

				// if mesh has no blas add new one
				if (!m_blasMap.count(mesh)) {
					vk::AccelerationStructure& accelerationStructure = m_blasMap[mesh] = vk::AccelerationStructure();
					accelerationStructure.setType(VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR);
					accelerationStructure.init();
					accelerationStructure.addGeometry(mesh->getVertexBuffer(), sizeof(Vertex), mesh->getIndexBuffer());
					accelerationStructure.update();
				}

				instanceVector.push_back(vk::AccelerationStructureInstance(m_blasMap.at(mesh)));
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

		VkWriteDescriptorSetAccelerationStructureKHR writeAccel{ VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET_ACCELERATION_STRUCTURE_KHR, nullptr };
		writeAccel.accelerationStructureCount = 1;
		writeAccel.pAccelerationStructures = m_tlas.getVkAccelerationStructureKHRptr();

		auto write = m_rtDescriptorSet->writeGeneric(&writeAccel, 1, 0);
		m_rtDescriptorSet->write(1, &write);
	}

	void PathTracer::afterRender() {
		m_frameIndex++;
	}

	void PathTracer::recordCommands(const vk::CommandBuffer* cmd) {
		vkCmdBindPipeline(*cmd, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, m_rtPipeline);
		std::array<VkDescriptorSet, 5> boundSets = {
			m_rtDescriptorSet,
			m_descriptorSet,
			m_textureSet,
			m_targetDescriptorSet,
			m_storageDescriptorSet
		};
		vkCmdBindDescriptorSets(*cmd, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, m_rtPipeline.getVkPipelineLayout(), 0, boundSets.size(), boundSets.data(), 0, nullptr);

		vkCmdTraceRaysKHR(*cmd,
			&m_rtPipeline.getRayGenRegion(),
			&m_rtPipeline.getMissRegion(),
			&m_rtPipeline.getHitRegion(),
			&m_rtPipeline.getCallRegion(),
			m_target->getExtent().width, m_target->getExtent().height, 1
		);

		m_target->recLayoutTransition(*cmd, VK_IMAGE_LAYOUT_GENERAL, m_targetFinalLayout, VK_ACCESS_SHADER_WRITE_BIT, VK_ACCESS_NONE);
	}

	void PathTracer::callback(const RenderEvent::Resize& event) {
		resetRender();
	}

	void PathTracer::callback(const AssetHandlerEvent::TextureLoad& event) {
		m_areTexturesOutdated = true;
	}

	void PathTracer::callback(const SceneEvent::AddModel& event) {
		resetRender();
	}

	void PathTracer::callback(const SceneEvent::RemoveModel& event) {
		resetRender();
	}

	void PathTracer::callback(const SceneEvent::AddLight& event) {
		auto radius = event.actor.cmpLight_getRadius();
		float aabbMin[3] = { -radius, -radius, -radius };
		float aabbMax[3] = { radius, radius, radius };
		vk::AccelerationStructure& accelerationStructure = (m_lightBlasMap[event.actor] = vk::AccelerationStructure());
		accelerationStructure.setType(VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR);
		accelerationStructure.init();
		accelerationStructure.addGeometry(aabbMin, aabbMax);
		accelerationStructure.update();

		resetRender();
	}

	void PathTracer::callback(const SceneEvent::RemoveLight& event) {
		vk::AccelerationStructure& accelerationStructure = m_lightBlasMap.at(event.actor);
		accelerationStructure.destroy();
		m_lightBlasMap.erase(event.actor);

		resetRender();
	}

	void PathTracer::callback(const SceneEvent::UpdateLightBuffer& event) {
		VkDescriptorBufferInfo lightBufferInfo{ *getSceneLightBuffer(), 0, getSceneLightBuffer()->getSize() };
		VkWriteDescriptorSet write = m_descriptorSet->writeBuffer(lightBufferInfo, 1);
		m_descriptorSet->write(1, &write);
	}

	void PathTracer::callback(const SceneEvent::UpdateMeshInstanceBuffer& event) {
		VkDescriptorBufferInfo perMeshBufferInfo{ *getScenePerMeshInstanceBuffer(), 0, getScenePerMeshInstanceBuffer()->getSize() };
		VkWriteDescriptorSet write = m_descriptorSet->writeBuffer(perMeshBufferInfo, 2);
		m_descriptorSet->write(1, &write);
	}

	void PathTracer::updateTextureDescriptor() {
		m_pRenderer->destroyDescriptorSet(m_textureSet);
		m_textureSet = m_pRenderer->createDescriptorSet<GenericDescriptorSet>();

		Base* base = Base::getBase();
		auto* textureMap = RenderTask::getTextureDataMap();
		DescriptorSetBinding texturesBinding(
			VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
			VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR,
			textureMap->size()
		);
		std::vector<VkDescriptorImageInfo> textureImageInfos(textureMap->size());
		for (auto& texturePair : *textureMap) {
			uint32_t i = RenderTask::getTextureIndex(texturePair.first);
			textureImageInfos[i] = { base->m_textureSampler, texturePair.second->getImage(), VK_IMAGE_LAYOUT_GENERAL};
		}

		m_textureSet->addBinding(texturesBinding);
		m_textureSet->createLayout();
		m_textureSet->allocate();

		auto textureWrite = m_textureSet->writeImage(textureImageInfos.data(), textureImageInfos.size(), 0);
		m_textureSet->write(1, &textureWrite);

		m_rtPipeline.setDescriptorSetLayout(2, m_textureSet->getLayout());
		m_rtPipeline.update();

		m_areTexturesOutdated = false;
	}
}