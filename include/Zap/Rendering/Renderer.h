#pragma once

#include "Zap/Zap.h"
#include "Zap/Rendering/Window.h"
#include "Zap/Vertex.h"
#include "Zap/Rendering/RenderTemplate.h"
#include "Zap/Scene/Camera.h"
#include "glm.hpp"

namespace Zap {
	class Renderer
	{
	public:
		Renderer(Window& window);
		~Renderer();
	
		void init();

		void destroy();

		//!Render all rendertemplates to the screen
		void render();

		void addRenderTemplate(RenderTemplate* renderTemplate);

	private:
		bool m_isInit = false;

		Window& m_window;

		//Surface & Swapchain
		vk::Surface m_surface = vk::Surface();
		uint32_t m_currentImageIndex = 0;
		vk::Swapchain m_swapchain = vk::Swapchain();

		//CommandBuffers
		uint32_t m_commandBufferCount;
		vk::CommandBuffer* m_commandBuffers;

		//Fences
		VkFence m_imageAvailable = VK_NULL_HANDLE;
		VkFence m_renderComplete = VK_NULL_HANDLE;

		std::vector<RenderTemplate*> m_renderTemplates;

		void recordCommandBuffers();

		friend class PBRenderer;//TODO add rendertoolkit for userdefined rendertemplates
		friend class Gui;
	};
}

