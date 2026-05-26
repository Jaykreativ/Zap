#pragma once

#include "imgui.h"

#include "Zap/Zap.h"
#include "Zap/Rendering/Image.h"
#include "Zap/Rendering/RenderObject.h"
#include "Zap/Rendering/RenderObjects/RenderTask.h"
#include "Zap/Rendering/RenderObjects/RenderTargets.h"
#include "Zap/Rendering/RenderObjects/Framebuffer.h"

#include <vector>

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
		Gui(Renderer* pRenderer, RenderTargetHandle<> target, Window* pWindow);
		~Gui();

		//GuiTexture loadTexture(Zap::Image* pImage);
		//GuiTexture loadTexture(const char* texturePath);
		//
		//void unloadTexture(GuiTexture texture);

		static void pushContext(RenderTaskHandle<Gui> handle);

		static void popContext();

		void enableClear() { m_shouldClear = true; }

		void disableClear(){ m_shouldClear = false; }

	private:
		bool m_shouldClear = false;

		ImGuiContext* m_context = nullptr;
		static std::vector<RenderTaskHandle<Gui>> m_contextStack;

		Window* m_pWindow;

		RenderTargetHandle<> m_target;

		VkDescriptorPool m_descriptorPool;
		vk::RenderPass m_renderPass;

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
	};
}