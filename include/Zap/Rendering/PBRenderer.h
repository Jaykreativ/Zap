#pragma once

#include "Zap/Zap.h"
#include "Zap/Scene/Actor.h"
#include "Zap/Scene/Material.h"
#include "Zap/Rendering/RenderTemplate.h"

#include "glm.hpp"

namespace Zap {
	class Scene;

	class PBRenderer : public RenderTemplate
	{
	public:
		PBRenderer(Renderer& renderer, Scene* pScene);
		PBRenderer(const PBRenderer& pbrenderer);
		~PBRenderer();

		void updateCamera(Actor camera);

		void changeScene(Scene* pScene); // TODO implement

		// Resizes framebuffers
		// Works only when rendering to a custom target
		void resize();

		void setRenderTarget(Image* target);

		void setDefaultRenderTarget();

		Image* getRenderTarget();

		void setViewport(uint32_t width, uint32_t height, uint32_t x, uint32_t y);

		void getViewport(uint32_t& width, uint32_t& height, uint32_t& x, uint32_t& y);

	private:
		bool m_isInit = false;

		Renderer& m_renderer;
		Scene* m_pScene = nullptr;
		Image* m_pTarget = nullptr;

		VkViewport m_viewport;
		VkRect2D m_scissor;

		vk::RenderPass m_renderPass = vk::RenderPass();

		vk::Image m_depthImage;

		std::vector<vk::Framebuffer> m_framebuffers;

		vk::DescriptorPool m_descriptorPool = vk::DescriptorPool();
		vk::DescriptorSet m_descriptorSet = vk::DescriptorSet();

		vk::Shader m_vertexShader = vk::Shader();
		vk::Shader m_fragmentShader = vk::Shader();
		vk::Pipeline m_pipeline = vk::Pipeline();

		//Semaphores
		VkSemaphore m_semaphoreRenderComplete;

		//Buffers
		struct UniformBufferObject {// definition of the uniform buffer layout
			glm::mat4 model;
			glm::mat4 modelNormal;
			glm::mat4 view;
			glm::mat4 perspective;
			alignas(16) glm::vec3 camPos;
			alignas(16) glm::vec3 color;
			alignas(4) uint32_t lightCount;
		};

		UniformBufferObject m_ubo{};// the host uniform buffer
		vk::Buffer m_uniformBuffer = vk::Buffer();// the vulkan uniform buffer

		struct LightData {
			alignas(16) glm::vec3 pos;
			alignas(16) glm::vec3 color;
		};

		vk::Buffer m_lightBuffer = vk::Buffer();

		struct PerMeshData {
			glm::mat4 transform;
			glm::mat4 normalTransform;
			Material material;
		};

		vk::Buffer m_perMeshBuffer;

		void onRendererInit();

		void destroy();

		void beforeRender();

		void afterRender();

		void recordCommands(const vk::CommandBuffer* cmd, uint32_t imageIndex);

		void onWindowResize(int width, int height);
	};
}

