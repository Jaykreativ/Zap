#pragma once

#include "Zap/Zap.h"

namespace Zap
{
	class Renderer;
	class Window
	{
	public:
		Window(uint32_t width, uint32_t height, std::string title);
		~Window();

		void init();

		void clear();

		void clearColor();

		void clearDepthStencil();

		void swapBuffers();

		bool shouldClose();

		void show();

		void setKeyCallback(GLFWkeyfun callback);

		void setResizeCallback(GLFWwindowsizefun callback);

		void recordClearCommandBuffers();

		void recordClearDepthStencilCommandBuffer();

		void resizeVk(GLFWwindow* window, int width, int height);

		void addRenderer(Renderer* renderer);

		/*Getter*/
		uint32_t getWidth();

		uint32_t getHeight();

		GLFWwindow* getGLFWwindow();

		vk::Surface* getSurface();

		vk::Swapchain* getSwapchain();

		vk::Image* Window::getDepthImage();

		vk::RenderPass* getRenderPass();

		vk::Framebuffer* getFramebuffer(uint32_t index);
		std::vector<vk::Framebuffer> getFramebuffers();
		vk::Framebuffer* getFramebufferPtr();

		uint32_t getCurrentImageIndex();

		VkFence getImageAvailableFence();

		Renderer* getRenderer(uint32_t index);

		static void pollEvents();

	private:
		bool m_isInit = false;

		uint32_t m_width;
		uint32_t m_height;
		std::string m_title;

		GLFWwindow *m_window;
		GLFWwindowsizefun m_sizeCallback;

		std::vector<Renderer*> m_renderers;

		vk::Surface m_surface = vk::Surface();
		vk::Swapchain m_swapchain = vk::Swapchain();
		vk::Image m_depthImage;
		vk::RenderPass m_renderPass = vk::RenderPass();
		std::vector<vk::Framebuffer> m_framebuffers;
		uint32_t m_currentImageIndex;
		VkFence m_imageAvailable = VK_NULL_HANDLE;

		vk::CommandBuffer* m_clearCommandBuffers;
		vk::CommandBuffer m_clearDepthStencilCommandBuffer = vk::CommandBuffer();

		friend class PBRenderer;
		friend class Gui;
	};
}
