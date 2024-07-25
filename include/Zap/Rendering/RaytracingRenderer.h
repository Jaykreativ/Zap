#pragma once

#include "Zap/Rendering/RenderTaskTemplate.h"
#include "glm.hpp"


namespace Zap {
	class Actor; //forward declaration

	class RaytracingRenderer : public RenderTaskTemplate
	{
	public:
		RaytracingRenderer(Scene* pScene);
		~RaytracingRenderer();

		void updateCamera(const Actor camera);

	private:
		Scene* m_pScene = nullptr;

		std::unordered_map<UUID, vk::AccelerationStructure> m_blasMap;
		vk::AccelerationStructure m_tlas;

		glm::vec2 m_extent = { 1, 1 };
		vk::Buffer m_UBO;

		vk::Shader m_rgenShader;
		vk::Shader m_rchitShader;
		vk::Shader m_rmissShader;
		vk::Shader m_rsmissShader;

		vk::DescriptorPool m_descriptorPool;
		vk::DescriptorSet m_rtDescriptorSet;
		vk::DescriptorSet m_descriptorSet;
		std::vector<vk::DescriptorSet> m_targetDescriptorSets = {};

		vk::RtPipeline m_rtPipeline;

		// TODO remove when useless
		uint32_t m_oldLigthbufferSize = 0;

		void init(uint32_t width, uint32_t height, uint32_t imageCount);

		void initTargetDependencies(uint32_t width, uint32_t height, uint32_t imageCount, vk::Image* pTarget, uint32_t imageIndex);

		void resize(uint32_t width, uint32_t height, uint32_t imageCount);

		void resizeTargetDependencies(uint32_t width, uint32_t height, uint32_t imageCount, vk::Image* pTarget, uint32_t imageIndex);

		void destroy();

		void beforeRender(vk::Image* pTarget, uint32_t imageIndex);

		void afterRender(vk::Image* pTarget, uint32_t imageIndex);

		void recordCommands(const vk::CommandBuffer* cmd, vk::Image* pTarget, uint32_t imageIndex);
	};
}

