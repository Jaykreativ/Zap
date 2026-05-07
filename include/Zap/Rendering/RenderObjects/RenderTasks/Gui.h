#pragma once

#include "Zap/Zap.h"
#include "Zap/Rendering/Image.h"
#include "Zap/Rendering/RenderObject.h"
#include "Zap/Rendering/RenderObjects/RenderTask.h"
#include "Zap/Rendering/RenderObjects/RenderTargets.h"
#include "Zap/Rendering/RenderObjects/Framebuffer.h"

namespace Zap {
	typedef VkDescriptorSet GuiTexture;

	// Enables usage of a Zap image in the Gui by passing this reference to the ImGui::Image() function
	// Does not work for renderTargetImage which are handled by the Renderer, use renderTargetGuiImage instead
	class GuiImageRef {
	public:
		GuiImageRef(std::weak_ptr<Image2D> imageRef);
		~GuiImageRef();

		operator bool();

		operator GuiTexture();

	private:
		std::weak_ptr<Image> m_imageRef;
		vk::Sampler m_texSampler;
		GuiTexture m_guiTexture;
	};

	class Gui : public RenderTask {
	public:
		Gui(Renderer* pRenderer, RenderTargetHandle<> target);
		~Gui();

		//GuiTexture loadTexture(Zap::Image* pImage);
		//GuiTexture loadTexture(const char* texturePath);
		//
		//void unloadTexture(GuiTexture texture);

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

		void init(const LayoutTransitionHelper& layoutTransitionHelper) override;

		void destroy() override;

		void beforeRender() override;

		void afterRender() override;

		void addDescriptorPoolSizes(DescriptorPoolSizeList& poolSizes) override;

		TaskLayoutTransitions getLayoutTransitions() override;

		void recordCommands(const vk::CommandBuffer* cmd) override;

		static bool isImGuiInit;
		static VkDescriptorPool descriptorPool;
		static vk::RenderPass renderPass;
		static Window* pImGuiWindow;
	};
}