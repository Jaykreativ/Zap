#pragma once

#include "Zap/Zap.h"
#include "Zap/Rendering/RenderObjects/RenderTask.h"
#include "Zap/Rendering/RenderObjects/RenderTargets.h"
#include "Zap/Rendering/RenderObjects/DescriptorSet.h"
#include "glm.hpp"

namespace Zap {
	class Actor;
	class AddLightEvent;
	class RemoveLightEvent;
	class AddModelEvent;
	class RemoveModelEvent;

	class PathTracer : public RenderTask
	{
	public:
		PathTracer(Renderer* pRenderer, RenderTargetHandle<> target, Scene* pScene);
		~PathTracer();

		void updateCamera(const Actor camera);

		void resetRender();

	private:
		Scene* m_pScene = nullptr;

		std::unordered_map<UUID, vk::AccelerationStructure> m_blasMap;
		std::unordered_map<UUID, vk::AccelerationStructure> m_lightBlasMap;
		vk::AccelerationStructure m_tlas;

		RenderTargetHandle<> m_target;

		RenderTargetHandle<RenderTargetImage> m_storageTarget;

		vk::Buffer m_UBO;
		uint32_t m_frameIndex = 0;

		vk::Shader m_rgenShader;
		vk::Shader m_rchitShader;
		vk::Shader m_rmissShader;
		vk::Shader m_rintShader;

		/* Set: 0
		* [0] AccelerationStructure
		*/
		DescriptorSetHandle<GenericDescriptorSet> m_rtDescriptorSet;
		/* Set: 1
		* [0] CamUBO
		* [1] LightBuffer
		* [2] PerMeshInstanceBuffer
		* [3] Textures
		*/
		DescriptorSetHandle<GenericDescriptorSet> m_descriptorSet;
		/* Set: 2
		* [0] Target
		*/
		DescriptorSetHandle<RenderTargetDescriptorSet> m_targetDescriptorSet;
		/* Set: 3
		* [0] StorageImage
		*/
		DescriptorSetHandle<RenderTargetDescriptorSet> m_storageDescriptorSet;

		vk::RtPipeline m_rtPipeline;

		// TODO remove when useless
		uint32_t m_oldLightbufferSize = 0;

		bool m_areTexturesOutdated = false;

		uint32_t m_loadedTextureCount = 0;

		void addDescriptorPoolSizes(DescriptorPoolSizeList& poolSizes) override;

		TaskLayoutTransitions getLayoutTransitions() override;

		void init(const LayoutTransitionHelper& layoutTransitionHelper) override;

		void destroy() override;

		void beforeRender() override;

		void afterRender() override;

		void recordCommands(const vk::CommandBuffer* cmd) override;

		void updateTextureDescriptor();
	};
}

