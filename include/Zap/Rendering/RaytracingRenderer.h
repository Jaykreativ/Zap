#pragma once

#include "Zap/Rendering/RenderTemplate.h"
#include "glm.hpp"


namespace Zap {
	class Actor; //forward declaration

	class RaytracingRenderer : public RenderTemplate
	{
	public:
		RaytracingRenderer(Renderer& renderer, Scene* pScene);
		~RaytracingRenderer();

		void updateCamera(const Actor camera);

		void resize();

		void setRenderTarget(Image* target);

		void setDefaultRenderTarget();

		Image* getRenderTarget();

	private:
		Renderer& m_renderer;
		Scene* m_pScene = nullptr;

		Image* m_pTarget = nullptr;

		std::unordered_map<uint32_t, vk::AccelerationStructure> m_blasMap;
		vk::AccelerationStructure m_tlas;
		glm::vec2 m_extent = {1, 1};
		vk::Buffer m_UBO;
		vk::Shader m_rgenShader;
		vk::Shader m_rchitShader;
		vk::Shader m_rmissShader;
		vk::Shader m_rsmissShader;
		vk::DescriptorPool m_descriptorPool;
		vk::DescriptorSet m_rtDescriptorSet;
		vk::DescriptorSet m_descriptorSet;
		vk::RtPipeline m_rtPipeline;

		// TODO remove when useless
		uint32_t m_oldLigthbufferSize = 0;

		void onRendererInit();

		void destroy();

		void beforeRender();

		void afterRender();

		void recordCommands(const vk::CommandBuffer* cmd, uint32_t imageIndex);

		void onWindowResize(int width, int height);
	};
}

