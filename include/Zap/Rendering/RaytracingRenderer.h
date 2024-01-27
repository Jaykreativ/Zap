#pragma once

#include "Zap/Rendering/RenderTemplate.h"
#include "glm.hpp"

namespace Zap {
	class RaytracingRenderer : public RenderTemplate
	{
	public:
		RaytracingRenderer(Renderer& renderer, Scene* pScene);
		~RaytracingRenderer();

		void setExtent(glm::vec2 extent);

		vk::Image& getOutputImage();

	private:
		Renderer& m_renderer;
		Scene* m_pScene = nullptr;

		std::unordered_map<uint32_t, vk::AccelerationStructure> m_blasVector;
		vk::AccelerationStructure m_tlas;
		glm::vec2 m_extent = {1, 1};
		vk::Image m_rtOutImage;
		vk::Shader m_rgenShader;
		vk::Shader m_rchitShader;
		vk::Shader m_rmissShader;
		vk::DescriptorPool m_rtDscriptorPool;
		vk::RtPipeline m_rtPipeline;

		void init();

		void destroy();

		void beforeRender();

		void afterRender();

		void recordCommands(const vk::CommandBuffer* cmd, uint32_t imageIndex);

		void resize(int width, int height);
	};
}

