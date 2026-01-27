#pragma once

#include "Zap/Zap.h"
#include "Zap/Events.h"

namespace Zap
{
	class Renderer;
	class Window
	{
	public:
		Window(uint32_t width, uint32_t height, std::string title);
		~Window();

		operator GLFWwindow* () { return m_window; }

		void init();

		void destroy();

		// Will push the next frame to the screen
		// Should be executed after rendering
		void present();

		bool shouldClose();

		bool isIconified();

		void show();

		WindowEventHandler& getEventHandler();

		/*Getter*/
		uint32_t getWidth();

		uint32_t getHeight();

		uint32_t getSwapchainImageIndex();

		const vk::Surface* getSurface();

		const vk::Swapchain* getSwapchain();

		static void pollEvents();

#ifndef ZP_ALL_PUBLIC
	private:
#endif

		bool m_isInit = false;

		uint32_t m_width;
		uint32_t m_height;
		std::string m_title;

		GLFWwindow *m_window;
		vk::Surface m_surface = vk::Surface();
		uint32_t m_currentSwapchainImageIndex = 0;
		VkFence m_imageAvailable = VK_NULL_HANDLE;
		vk::Swapchain m_swapchain = vk::Swapchain();

		GLFWwindowsizefun m_sizeCallback = nullptr;

		WindowEventHandler m_eventHandler;

		void resize(GLFWwindow* window, int width, int height);

		// maps glfw window ptr to the corresponding zap window ptr
		static std::unordered_map<GLFWwindow*, Window*> glfwWindowMap;
		static void resizeGLFWCallback(GLFWwindow* window, int width, int height);
		static void keyGLFWCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
		static void cursorPosGLFWCallback(GLFWwindow* window, double xPos, double yPos);
		static void mouseButtonGLFWCallback(GLFWwindow* window, int button, int action, int mods);
		static void scrollGLFWCallback(GLFWwindow* window, double xoffset, double yoffset);
		static void dragDropGLFWCallback(GLFWwindow* window, int path_count, const char* paths[]);

		friend class Renderer;
		friend class PBRenderer;
		friend class Gui;
	};
}