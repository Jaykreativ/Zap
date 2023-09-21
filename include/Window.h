#pragma once

#include "Zap.h"

namespace Zap
{
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

		void setKeyCallback(GLFWkeyfun callback);

		void show();

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

		static void pollEvents();

	private:
		bool m_isInit = false;

		uint32_t m_width;
		uint32_t m_height;
		std::string m_title;

		GLFWwindow *m_window;

		vk::Surface m_surface = vk::Surface();
		vk::Swapchain m_swapchain = vk::Swapchain();
		vk::Image m_depthImage;
		vk::RenderPass m_renderPass = vk::RenderPass();
		std::vector<vk::Framebuffer> m_framebuffers;
		uint32_t m_currentImageIndex;
		VkFence m_imageAvailable = VK_NULL_HANDLE;

		vk::CommandBuffer m_clearCommandBuffer = vk::CommandBuffer();
	};
}
