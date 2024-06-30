#pragma once

#include "Zap/Zap.h"
#include "Zap/Rendering/RenderTemplate.h"
#include "glm.hpp"

namespace Zap {
	class Actor;
	class AddLightEvent;
	class RemoveLightEvent;
	class AddModelEvent;
	class RemoveModelEvent;

	class PathTracer : public RenderTemplate
	{
	public:
		PathTracer(Renderer& renderer, Scene* pScene);
		~PathTracer();

		void updateCamera(const Actor camera);

		void resize();

		void resetRender();

		void setRenderTarget(Image* target);

		void setDefaultRenderTarget();

		Image* getRenderTarget();

	private:
		Renderer& m_renderer;
		Scene* m_pScene = nullptr;

		Image* m_pTarget = nullptr;

		std::unordered_map<uint32_t, vk::AccelerationStructure> m_blasMap;
		std::unordered_map<UUID, vk::AccelerationStructure> m_lightBlasMap;
		vk::AccelerationStructure m_tlas;
		vk::Image m_storageImage;
		glm::vec2 m_extent = { 1, 1 };
		vk::Buffer m_UBO;
		uint32_t m_frameIndex = 0;
		vk::Shader m_rgenShader;
		vk::Shader m_rchitShader;
		vk::Shader m_rmissShader;
		vk::Shader m_rintShader;
		vk::DescriptorPool m_descriptorPool;
		vk::DescriptorSet m_rtDescriptorSet;
		vk::DescriptorSet m_descriptorSet;
		vk::RtPipeline m_rtPipeline;

		// TODO remove when useless
		uint32_t m_oldLightbufferSize = 0;

		void onRendererInit();

		void destroy();
		
		void beforeRender();
		
		void afterRender();
		
		void recordCommands(const vk::CommandBuffer* cmd, uint32_t imageIndex);
		
		void onWindowResize(int width, int height);

		static void addLightCallback(AddLightEvent& eventParams, void* customParams);

		static void removeLightCallback(RemoveLightEvent& eventParams, void* customParams);

		static void addModelCallback(AddModelEvent& eventParams, void* customParams);

		static void removeModelCallback(RemoveModelEvent& eventParams, void* customParams);
	};
}

