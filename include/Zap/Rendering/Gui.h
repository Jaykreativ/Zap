#pragma once

#include "Zap/Zap.h"
#include "Zap/Rendering/RenderTaskTemplate.h"

namespace Zap {
	typedef VkDescriptorSet GuiTexture;

	class Gui : public RenderTaskTemplate {
	public:
		Gui();
		~Gui();

		GuiTexture loadTexture(Zap::Image* pImage);
		GuiTexture loadTexture(const char* texturePath);

		void unloadTexture(GuiTexture texture);

		void enableClear() { m_shouldClear = true; }

		void disableClear(){ m_shouldClear = false; }

		static void initImGui(Window* pWindow);

		static void destroyImGui();

	private:
		bool m_shouldClear = false;// TODO implement enable/disable Clear in Gui (make Gui a singleton)

		uint32_t m_framebufferCount = 0;
		vk::Framebuffer* m_framebuffers = nullptr;

		vk::Sampler m_textureSampler;
		std::vector<vk::Image> m_textures = {};

		void init(uint32_t width, uint32_t height, uint32_t imageCount);

		void initTargetDependencies(uint32_t width, uint32_t height, uint32_t imageCount, vk::Image* pTarget, uint32_t imageIndex);

		void resize(uint32_t width, uint32_t height, uint32_t imageCount);

		void resizeTargetDependencies(uint32_t width, uint32_t height, uint32_t imageCount, vk::Image* pTarget, uint32_t imageIndex);

		void destroy();

		void beforeRender(vk::Image* pTarget, uint32_t imageIndex);

		void afterRender(vk::Image* pTarget, uint32_t imageIndex);

		void recordCommands(const vk::CommandBuffer* cmd, vk::Image* pTarget, uint32_t imageIndex);

		static bool isImGuiInit;
		static VkDescriptorPool descriptorPool;
		static vk::RenderPass renderPass;
		static Window* pImGuiWindow;
	};
}