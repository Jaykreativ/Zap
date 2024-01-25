#include "Zap/Rendering/RaytracingRenderer.h"
#include "Zap/Rendering/Renderer.h"
#include "Zap/Scene/Scene.h"
#include "Zap/Scene/Model.h"
#include "Zap/Scene/Mesh.h"

namespace Zap {
	RaytracingRenderer::RaytracingRenderer(Renderer& renderer, Scene* pScene)
		: m_renderer(renderer), m_pScene(pScene)
	{}

	RaytracingRenderer::~RaytracingRenderer() {}

	void RaytracingRenderer::init() {
		for (uint32_t id : m_pScene->m_meshReferences) {
			Mesh& mesh = Mesh::all[id];
			vk::AccelerationStructure& accelerationStructure = m_blasVector[id] = vk::AccelerationStructure();
			accelerationStructure.setType(VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR);
			accelerationStructure.addGeometry(mesh.m_vertexBuffer, sizeof(Vertex), mesh.m_indexBuffer);
			accelerationStructure.init();
		}
		std::vector<vk::AccelerationStructureInstance> instanceVector;
		for (auto const& modelPair : m_pScene->m_modelComponents) {
			glm::mat4* transform = &m_pScene->m_transformComponents.at(modelPair.first).transform;
			for (uint32_t id : modelPair.second.m_meshes) {
				instanceVector.push_back(vk::AccelerationStructureInstance(m_blasVector.at(id)));
				instanceVector.back().setTransform(*((VkTransformMatrixKHR*)transform));
			}
		}
		m_tlas = vk::AccelerationStructure();
		m_tlas.setType(VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR);
		m_tlas.addGeometry(instanceVector);
		m_tlas.init();

		m_rtOutImage.setFormat(VK_USED_SCREENCOLOR_FORMAT);
		m_rtOutImage.setAspect(VK_IMAGE_ASPECT_COLOR_BIT);
		m_rtOutImage.setUsage(VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT);
		m_rtOutImage.setInitialLayout(VK_IMAGE_LAYOUT_PREINITIALIZED);
		m_rtOutImage.setWidth(m_extent.x);
		m_rtOutImage.setHeight(m_extent.y);
		
		m_rtOutImage.init();
		m_rtOutImage.allocate(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
		m_rtOutImage.initView();
		
		m_rtOutImage.changeLayout(VK_IMAGE_LAYOUT_GENERAL, VK_ACCESS_TRANSFER_WRITE_BIT | VK_ACCESS_SHADER_WRITE_BIT);

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

		group.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_KHR;
		group.generalShader = VK_SHADER_UNUSED_KHR;
		group.closestHitShader = 1;

		m_rtPipeline.addGroup(group);

		group.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
		group.generalShader = 2;
		group.closestHitShader = VK_SHADER_UNUSED_KHR;

		m_rtPipeline.addGroup(group);

		m_rtDscriptorPool.addDescriptorSet();

		VkWriteDescriptorSetAccelerationStructureKHR accelerationStructureDescriptor{ VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET_ACCELERATION_STRUCTURE_KHR };
		accelerationStructureDescriptor.accelerationStructureCount = 1;
		VkAccelerationStructureKHR accelerationStructure = m_tlas;
		accelerationStructureDescriptor.pAccelerationStructures = &accelerationStructure;

		vk::Descriptor descriptor{};
		descriptor.pNext = &accelerationStructureDescriptor;
		descriptor.type = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR;
		descriptor.stages = VK_SHADER_STAGE_RAYGEN_BIT_KHR;
		descriptor.binding = 0;

		m_rtDscriptorPool.addDescriptor(descriptor, 0);

		VkDescriptorImageInfo imageInfo{};
		imageInfo.sampler = VK_NULL_HANDLE;
		imageInfo.imageLayout = m_rtOutImage.getLayout();
		imageInfo.imageView = m_rtOutImage.getVkImageView();

		descriptor = {};
		descriptor.type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
		descriptor.stages = VK_SHADER_STAGE_RAYGEN_BIT_KHR;
		descriptor.binding = 1;
		descriptor.pImageInfo = &imageInfo;

		m_rtDscriptorPool.addDescriptor(descriptor, 0);

		m_rtDscriptorPool.init();

		m_rtPipeline.addDescriptorSetLayout(m_rtDscriptorPool.getVkDescriptorSetLayout(0));
		m_rtPipeline.init(); m_rtPipeline.initShaderBindingTable();
	}

	void RaytracingRenderer::destroy() {
		m_rtPipeline.destroy();
		m_rtDscriptorPool.~DescriptorPool();
		m_rgenShader.~Shader();
		m_rchitShader.~Shader();
		m_rmissShader.~Shader();
		m_rtOutImage.destroy();
		m_tlas.destroy();
		for (auto& blasPair : m_blasVector) {
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
			m_rtDscriptorPool.getVkDescriptorSet(0) 
		};
		vkCmdBindDescriptorSets(*cmd, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, m_rtPipeline.getVkPipelineLayout(), 0, boundSets.size(), boundSets.data(), 0, nullptr);

		vkCmdTraceRaysKHR(*cmd, &m_rtPipeline.getRayGenRegion(), &m_rtPipeline.getMissRegion(), &m_rtPipeline.getHitRegion(), &m_rtPipeline.getCallRegion(), m_extent.x, m_extent.y, 1);
	}

	void RaytracingRenderer::resize(int width, int height) {
		m_extent = { width, height };
	}

	void RaytracingRenderer::setExtent(glm::vec2 extent) {
		m_extent = extent;
	}
}