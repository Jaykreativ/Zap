#pragma once

#include "Zap/Zap.h"
#include "Zap/Rendering/RenderTemplate.h"

namespace Zap {
	typedef VkDescriptorSet GuiTexture;

	class Gui : public RenderTemplate {
	public:
		Gui(Renderer& renderer);
		~Gui();

		void init();

		GuiTexture loadTexture(const char* texturePath);

		void unloadTexture(GuiTexture texture);

		// wip
		void setRenderTarget(Image* target) {};// TODO add target functionality to gui

		// wip
		void setDefaultRenderTarget() {};

		// wip
		Image* getRenderTarget() { return nullptr; };

	private:
		bool m_isInit = false;

		Renderer& m_renderer;

		VkDescriptorPool m_imguiPool;

		vk::RenderPass m_renderPass = vk::RenderPass();
		uint32_t m_framebufferCount = 0;
		vk::Framebuffer* m_framebuffers = nullptr;

		vk::Sampler m_textureSampler;
		std::vector<vk::Image> m_textures = {};

		void onRendererInit();

		void destroy();

		void beforeRender();

		void afterRender();

		void recordCommands(const vk::CommandBuffer* cmd, uint32_t imageIndex);

		void onWindowResize(int width, int height);
	};
}