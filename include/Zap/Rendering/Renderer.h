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

		//Render all rendertemplates
		void render();

		void update();

		void beginRecord();

		void endRecord();

		void recRenderTemplate(RenderTemplate* pRenderTemplate);

		void recChangeImageLayout(Image* pImage, VkImageLayout layout, VkAccessFlags accessMask);

		void addRenderTemplate(RenderTemplate* pRenderTemplate);

#ifndef ZP_ALL_PUBLIC
	private:
#endif
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

		//Recording
		enum FunctionType {
			eRENDER_TEMPLATE = 0,
			eCHANGE_IMAGE_LAYOUT = 1
		};

		std::vector<FunctionType> m_recordedFunctions;
		std::vector<char> m_recordedParams = {};

		void recordCommandBuffers();

		void onWindowResize(int width, int height);

		friend class Window;
		friend class PBRenderer;//TODO add rendertoolkit for userdefined rendertemplates
		friend class Gui;
	};
}