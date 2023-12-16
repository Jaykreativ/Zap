#include "Zap/Rendering/Window.h"
#include "Zap/Rendering/Renderer.h"
#include "Zap/Rendering/PBRenderer.h"

namespace Zap {
	Window::Window(uint32_t width, uint32_t height, std::string title)
		: m_width(width), m_height(height), m_title(title)
	{}

	Window::~Window() {
		if (!m_isInit) return;
		m_isInit = false;
		//TODO destroy renderer
		glfwDestroyWindow(m_window);
	}

	void Window::init() {
		if (m_isInit) return;
		m_isInit = true;

		Zap::objects::windows.push_back(this);

		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		//glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
		glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
		m_window = glfwCreateWindow(m_width, m_height, m_title.c_str(), nullptr, nullptr);

		glfwSetWindowSizeCallback(m_window, resizeCallback);
	}

	bool Window::shouldClose() {
		return glfwWindowShouldClose(m_window);
	}

	void Window::show() {
		glfwShowWindow(m_window);
	}

	void Window::setKeyCallback(GLFWkeyfun callback) {
		glfwSetKeyCallback(m_window, callback);
	}

	void Window::setMousebButtonCallback(GLFWmousebuttonfun mouseButtonCallback) {
		glfwSetMouseButtonCallback(m_window, mouseButtonCallback);
	}

	void Window::setCursorPosCallback(GLFWcursorposfun cursorPosCallback) {
		glfwSetCursorPosCallback(m_window, cursorPosCallback);
	}

	void Window::setResizeCallback(GLFWwindowsizefun callback) {
		m_sizeCallback = callback;
	}

	void Window::resizeCallback(GLFWwindow* window, int width, int height) {
		for (uint32_t i = 0; i < Zap::objects::windows.size(); i++) {
			if (Zap::objects::windows[i]->m_window == window) {
				Zap::objects::windows[i]->resizeVk(window, width, height);
			}
		}
	}

	void Window::resizeVk(GLFWwindow* window, int width, int height) {
		m_width = width;
		m_height = height;

		if (m_height <= 0) return;

		vk::deviceWaitIdle();

		if (m_sizeCallback != nullptr) {
			m_sizeCallback(m_window, width, height);
		}

		/*
		m_swapchain.setWidth(width);
		m_swapchain.setHeight(height);
		m_swapchain.update();

		m_depthImage.setWidth(width);
		m_depthImage.setHeight(height);
		m_depthImage.update();

		m_depthImage.changeLayout(VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, 0, VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT);

		for (uint32_t i = 0; i < m_swapchain.getImageCount(); i++) {
			m_framebuffers[i].setWidth(width);
			m_framebuffers[i].setHeight(height);
			m_framebuffers[i].delAttachment(0);
			m_framebuffers[i].delAttachment(0);
			m_framebuffers[i].addAttachment(m_swapchain.getImageView(i));
			m_framebuffers[i].addAttachment(m_depthImage.getVkImageView());
			m_framebuffers[i].update();
		}

		recordClearCommandBuffers();
		recordClearDepthStencilCommandBuffer();

		for (Renderer* renderer : m_renderers) {
			renderer->recordCommandBuffers();
		}

		vk::acquireNextImage(m_swapchain, VK_NULL_HANDLE, m_imageAvailable, &m_currentImageIndex);
		vk::waitForFence(m_imageAvailable);*/
	}

	/*Getter*/
	uint32_t Window::getWidth() {
		return m_width;
	}
	uint32_t Window::getHeight() {
		return m_height;
	}

	void Window::pollEvents() {
		glfwPollEvents();
	}
}