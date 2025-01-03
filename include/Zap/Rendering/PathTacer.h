#pragma once

#include "Zap/Zap.h"
#include "Zap/Rendering/RenderTaskTemplate.h"
#include "glm.hpp"

namespace Zap {
	class Actor;
	class AddLightEvent;
	class RemoveLightEvent;
	class AddModelEvent;
	class RemoveModelEvent;

	class PathTracer : public RenderTaskTemplate
	{
	public:
		PathTracer(Scene* pScene);
		~PathTracer();

		void updateCamera(const Actor camera);

		void resetRender();

	private:
		Scene* m_pScene = nullptr;

		std::unordered_map<UUID, vk::AccelerationStructure> m_blasMap;
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
		/* Set: 0
		* [0] AccelerationStructure
		* [1] StorageImage
		*/
		vk::DescriptorSet m_rtDescriptorSet;
		/* Set: 1
		* [0] CamUBO
		* [1] LightBuffer
		* [2] PerMeshInstanceBuffer
		* [3] Textures
		*/
		vk::DescriptorSet m_descriptorSet;
		/* Set: 2
		* [0] Target
		*/
		std::vector<vk::DescriptorSet> m_targetDescriptorSets;

		vk::RtPipeline m_rtPipeline;

		// TODO remove when useless
		uint32_t m_oldLightbufferSize = 0;

		bool m_areTexturesOutdated = false;

		uint32_t m_loadedTextureCount = 0;

		void init(uint32_t width, uint32_t height, uint32_t imageCount);

		void initTargetDependencies(uint32_t width, uint32_t height, uint32_t imageCount, vk::Image* pTarget, uint32_t imageIndex);

		void resize(uint32_t width, uint32_t height, uint32_t imageCount);

		void resizeTargetDependencies(uint32_t width, uint32_t height, uint32_t imageCount, vk::Image* pTarget, uint32_t imageIndex);

		void destroy();

		void beforeRender(vk::Image* pTarget, uint32_t imageIndex);

		void afterRender(vk::Image* pTarget, uint32_t imageIndex);

		void recordCommands(const vk::CommandBuffer* cmd, vk::Image* pTarget, uint32_t imageIndex);

		void updateTextureDescriptor();

		static void textureLoadCallback(Zap::TextureLoadEvent& eventParams, void* customParams);

		static void addLightCallback(AddLightEvent& eventParams, void* customParams);

		static void removeLightCallback(RemoveLightEvent& eventParams, void* customParams);

		static void addModelCallback(AddModelEvent& eventParams, void* customParams);

		static void removeModelCallback(RemoveModelEvent& eventParams, void* customParams);
	};
}

