#pragma once

#include "Zap/Zap.h"
#include "Zap/Rendering/RenderTask.h"
#include "Zap/Rendering/RenderTargets.h"
#include "Zap/Rendering/Framebuffer.h"

namespace Zap {
	typedef VkDescriptorSet GuiTexture;

	class GuiImage : public Image {
	public:
		GuiImage();
		GuiImage(VkImage image);
		~GuiImage();

		void initView();

		void destroyView();

		void update();

		operator VkDescriptorSet() { return m_guiTexture; }

	private:
		vk::Sampler m_texSampler;
		VkDescriptorSet m_guiTexture;
	};

	class Gui : public RenderTask {
	public:
		Gui(RenderTargetHandle<> target);
		~Gui();

		GuiTexture loadTexture(Zap::Image* pImage);
		GuiTexture loadTexture(const char* texturePath);

		void unloadTexture(GuiTexture texture);

		void enableClear() { m_shouldClear = true; }

		void disableClear(){ m_shouldClear = false; }

		static void initImGui(Window* pWindow);

		static void destroyImGui();

	private:
		bool m_shouldClear = false;// TODO make Gui a singleton

		RenderTargetHandle<> m_target;

		FramebufferHandle m_framebuffer;

		vk::Sampler m_textureSampler;
		std::vector<vk::Image> m_textures = {};

		void init() override;

		void destroy() override;

		void beforeRender() override;

		void afterRender() override;

		void recordCommands(const vk::CommandBuffer* cmd) override;

		static bool isImGuiInit;
		static VkDescriptorPool descriptorPool;
		static vk::RenderPass renderPass;
		static Window* pImGuiWindow;
	};
}