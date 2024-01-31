#include "Zap/Rendering/RaytracingRenderer.h"
#include "Zap/Rendering/Renderer.h"
#include "Zap/Scene/Scene.h"
#include "Zap/Scene/Actor.h"
#include "Zap/Scene/Camera.h"
#include "Zap/Scene/Model.h"
#include "Zap/Scene/Mesh.h"

struct PerMeshData {// TODO make shared buffers in scene
	VkDeviceAddress vertexBufferAddress;
	VkDeviceAddress indexBufferAddress;
};

struct CameraUBO {
	glm::mat4 inverseView = glm::mat4(1);
	glm::mat4 inversePerspective = glm::mat4(1);
	uint32_t lightCount = 0;
};

struct LightData {
	alignas(16) glm::vec3 pos;
	alignas(16) glm::vec3 color;
};

namespace Zap {
	RaytracingRenderer::RaytracingRenderer(Renderer& renderer, Scene* pScene)
		: m_renderer(renderer), m_pScene(pScene)
	{}

	RaytracingRenderer::~RaytracingRenderer() {}

	void RaytracingRenderer::init() {
		std::unordered_map<uint32_t, uint32_t> idMap;// maps id to index of mesh in perMeshBuffer
		uint32_t i = 0;
		for (uint32_t id : m_pScene->m_meshReferences) {
			Mesh& mesh = Mesh::all[id];
			if (m_blasMap.count(id)) continue;
			vk::AccelerationStructure& accelerationStructure = m_blasMap[id] = vk::AccelerationStructure();
			accelerationStructure.setType(VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR);
			accelerationStructure.addGeometry(mesh.m_vertexBuffer, sizeof(Vertex), mesh.m_indexBuffer);
			accelerationStructure.init();
			idMap[id] = i;
			i++;
		}
		std::vector<vk::AccelerationStructureInstance> instanceVector;
		for (auto const& modelPair : m_pScene->m_modelComponents) {
			glm::mat4* transform = &glm::transpose(m_pScene->m_transformComponents.at(modelPair.first).transform);
			for (uint32_t id : modelPair.second.m_meshes) {
				instanceVector.push_back(vk::AccelerationStructureInstance(m_blasMap.at(id)));
				instanceVector.back().setTransform(*((VkTransformMatrixKHR*)transform));
				instanceVector.back().setCustomIndex(idMap.at(id));
			}
		}
		m_tlas = vk::AccelerationStructure();
		m_tlas.setType(VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR);
		m_tlas.addGeometry(instanceVector);
		m_tlas.init();


		m_rtOutImage.setFormat(VK_USED_SCREENCOLOR_FORMAT);
		m_rtOutImage.setAspect(VK_IMAGE_ASPECT_COLOR_BIT);
		m_rtOutImage.setUsage(VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);
		m_rtOutImage.setInitialLayout(VK_IMAGE_LAYOUT_PREINITIALIZED);
		m_rtOutImage.setWidth(m_extent.x);
		m_rtOutImage.setHeight(m_extent.y);
		
		m_rtOutImage.init();
		m_rtOutImage.allocate(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
		m_rtOutImage.initView();
		
		m_rtOutImage.changeLayout(VK_IMAGE_LAYOUT_GENERAL, VK_ACCESS_TRANSFER_WRITE_BIT | VK_ACCESS_SHADER_WRITE_BIT);

		m_perMeshBuffer = vk::Buffer(sizeof(PerMeshData)*m_blasMap.size(), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);
		m_perMeshBuffer.init(); m_perMeshBuffer.allocate(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
		{
			vk::Buffer perMeshStaging = vk::Buffer(m_perMeshBuffer.getSize(), VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
			perMeshStaging.init(); perMeshStaging.allocate(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
			void* rawData; perMeshStaging.map(&rawData);
			PerMeshData* data = (PerMeshData*)rawData;
			uint32_t i = 0;
			for (auto const& blasPair : m_blasMap) {
				auto& mesh = Mesh::all[blasPair.first];
				data[i].vertexBufferAddress = mesh.getVertexBuffer()->getVkDeviceAddress();
				data[i].indexBufferAddress = mesh.getIndexbuffer()->getVkDeviceAddress();
				i++;
			}
			perMeshStaging.unmap();
			m_perMeshBuffer.uploadData(&perMeshStaging);
			perMeshStaging.destroy();
		}

		m_camUBO = vk::Buffer(sizeof(CameraUBO), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
		m_camUBO.init(); m_camUBO.allocate(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
		{
			void* rawData; m_camUBO.map(&rawData);
			CameraUBO* data = (CameraUBO*)rawData;
			*data = CameraUBO();
			m_camUBO.unmap();
		}

		m_lightBuffer = vk::Buffer(sizeof(LightData) * std::max<size_t>(m_pScene->m_lightComponents.size(), 1), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
		m_lightBuffer.init(); m_lightBuffer.allocate(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

#ifdef _DEBUG
		vk::Shader::compile("../Zap/Shader/src/", { "raytrace.rgen", "raytrace.rchit", "raytrace.rmiss"}, {"./"});
#endif

		m_rgenShader.setPath("raytrace.rgen.spv");
		m_rchitShader.setPath("raytrace.rchit.spv");
		m_rmissShader.setPath("raytrace.rmiss.spv");

		m_rgenShader.setStage(VK_SHADER_STAGE_RAYGEN_BIT_KHR);
		m_rchitShader.setStage(VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR);
		m_rmissShader.setStage(VK_SHADER_STAGE_MISS_BIT_KHR);

		m_rgenShader.init();
		m_rchitShader.init();
		m_rmissShader.init();

		m_rtPipeline.addShader(m_rgenShader.getShaderStage());
		m_rtPipeline.addShader(m_rchitShader.getShaderStage());
		m_rtPipeline.addShader(m_rmissShader.getShaderStage());

		VkRayTracingShaderGroupCreateInfoKHR group{ VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR };
		group.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
		group.generalShader = 0;
		group.closestHitShader = VK_SHADER_UNUSED_KHR;
		group.anyHitShader = VK_SHADER_UNUSED_KHR;
		group.intersectionShader = VK_SHADER_UNUSED_KHR;

		m_rtPipeline.addGroup(group);

		group.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
		group.generalShader = 2;
		group.closestHitShader = VK_SHADER_UNUSED_KHR;

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
		descriptor.stages = VK_SHADER_STAGE_RAYGEN_BIT_KHR;
		descriptor.binding = 0;

		m_rtDescriptorSet.addDescriptor(descriptor);

		VkDescriptorImageInfo imageInfo{};
		imageInfo.sampler = VK_NULL_HANDLE;
		imageInfo.imageLayout = m_rtOutImage.getLayout();
		imageInfo.imageView = m_rtOutImage.getVkImageView();

		descriptor = {};
		descriptor.type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
		descriptor.stages = VK_SHADER_STAGE_RAYGEN_BIT_KHR;
		descriptor.binding = 1;
		descriptor.pImageInfo = &imageInfo;

		m_rtDescriptorSet.addDescriptor(descriptor);

		VkDescriptorBufferInfo perMeshBufferInfo{};
		perMeshBufferInfo.offset = 0;
		perMeshBufferInfo.range = m_perMeshBuffer.getSize();
		perMeshBufferInfo.buffer = m_perMeshBuffer;

		vk::Descriptor perMeshBufferDescriptor{};
		perMeshBufferDescriptor.type =	VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		perMeshBufferDescriptor.stages = VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;
		perMeshBufferDescriptor.binding = 2;
		perMeshBufferDescriptor.pBufferInfo = &perMeshBufferInfo;

		m_rtDescriptorSet.addDescriptor(perMeshBufferDescriptor);

		VkDescriptorBufferInfo camUBOInfo{};
		camUBOInfo.offset = 0;
		camUBOInfo.range = m_camUBO.getSize();
		camUBOInfo.buffer = m_camUBO;

		vk::Descriptor camUBODescriptor{};
		camUBODescriptor.binding = 0;
		camUBODescriptor.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		camUBODescriptor.stages = VK_SHADER_STAGE_RAYGEN_BIT_KHR | VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;
		camUBODescriptor.pBufferInfo = &camUBOInfo;

		m_descriptorSet.addDescriptor(camUBODescriptor);

		VkDescriptorBufferInfo lightBufferInfo{};
		lightBufferInfo.offset = 0;
		lightBufferInfo.range = m_lightBuffer.getSize();
		lightBufferInfo.buffer = m_lightBuffer;

		vk::Descriptor lightBufferDescriptor{};
		lightBufferDescriptor.binding = 1;
		lightBufferDescriptor.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		lightBufferDescriptor.stages = VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;
		lightBufferDescriptor.pBufferInfo = &lightBufferInfo;

		m_descriptorSet.addDescriptor(lightBufferDescriptor);

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
		m_rtOutImage.destroy();
		m_camUBO.destroy();
		m_perMeshBuffer.destroy();
		m_lightBuffer.destroy();
		m_tlas.destroy();
		for (auto& blasPair : m_blasMap) {
			blasPair.second.destroy();
		}
	}

	void RaytracingRenderer::beforeRender() {

	}

	void RaytracingRenderer::afterRender() {

	}

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
		void* rawData; m_camUBO.map(&rawData);
		CameraUBO* data = (CameraUBO*)rawData;
		data->inverseView = glm::inverse(camera.cmpCamera_getView());
		data->inversePerspective = glm::inverse(camera.cmpCamera_getPerspective(m_extent.x/m_extent.y));
		data->lightCount = m_pScene->m_lightComponents.size();
		m_camUBO.unmap();

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

		std::unordered_map<uint32_t, uint32_t> idMap;// maps id to index of mesh in perMeshBuffer
		uint32_t i = 0;
		for (uint32_t id : m_pScene->m_meshReferences) {
			if (idMap.count(id)) continue;
			idMap[id] = i;
			i++;
		}
		std::vector<vk::AccelerationStructureInstance> instanceVector;
		for (auto const& modelPair : m_pScene->m_modelComponents) {
			glm::mat4* transform = &glm::transpose(m_pScene->m_transformComponents.at(modelPair.first).transform);
			for (uint32_t id : modelPair.second.m_meshes) {
				instanceVector.push_back(vk::AccelerationStructureInstance(m_blasMap.at(id)));
				instanceVector.back().setTransform(*((VkTransformMatrixKHR*)transform));
				instanceVector.back().setCustomIndex(idMap.at(id));
			}
		}

		m_tlas.updateGeometry(instanceVector);
		m_tlas.update();
	}

	void RaytracingRenderer::resize(int width, int height) {
		if (width <= 0) width = 1;
		if (height <= 0) height = 1;
		m_extent = {width, height};
		m_rtOutImage.setWidth(width);
		m_rtOutImage.setHeight(height);
		m_rtOutImage.update();

		auto desc = m_rtDescriptorSet.getDescriptor(1);
		VkDescriptorImageInfo imageInfo{};
		imageInfo.imageView = m_rtOutImage.getVkImageView();
		imageInfo.imageLayout = m_rtOutImage.getLayout();
		desc.pImageInfo = &imageInfo;
		m_rtDescriptorSet.setDescriptor(1, desc);
		m_rtDescriptorSet.updateDescriptor(1);
	}

	vk::Image& RaytracingRenderer::getOutputImage() {
		return m_rtOutImage;
	}
}