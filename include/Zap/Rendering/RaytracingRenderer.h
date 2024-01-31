#pragma once

#include "Zap/Rendering/RenderTemplate.h"
#include "glm.hpp"

class Actor; //forward declaration

namespace Zap {
	class RaytracingRenderer : public RenderTemplate
	{
	public:
		RaytracingRenderer(Renderer& renderer, Scene* pScene);
		~RaytracingRenderer();

		void updateCamera(const Actor camera);

		void resize(int width, int height);

		vk::Image& getOutputImage();

	private:
		Renderer& m_renderer;
		Scene* m_pScene = nullptr;

		std::unordered_map<uint32_t, vk::AccelerationStructure> m_blasMap;
		vk::AccelerationStructure m_tlas;
		glm::vec2 m_extent = {1, 1};
		vk::Image m_rtOutImage;
		vk::Buffer m_perMeshBuffer;
		vk::Buffer m_camUBO;
		vk::Buffer m_lightBuffer = vk::Buffer();
		vk::Shader m_rgenShader;
		vk::Shader m_rchitShader;
		vk::Shader m_rmissShader;
		vk::DescriptorPool m_descriptorPool;
		vk::DescriptorSet m_rtDescriptorSet;
		vk::DescriptorSet m_descriptorSet;
		vk::RtPipeline m_rtPipeline;

		void init();

		void destroy();

		void beforeRender();

		void afterRender();

		void recordCommands(const vk::CommandBuffer* cmd, uint32_t imageIndex);

		void onWindowResize(int width, int height);
	};
}

