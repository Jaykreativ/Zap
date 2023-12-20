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
				Zap::objects::windows[i]->resize(window, width, height);
			}
		}
	}

	void Window::resize(GLFWwindow* window, int width, int height) {
		m_width = width;
		m_height = height;

		if (m_height <= 0) return;

		vk::deviceWaitIdle();

		if (m_sizeCallback != nullptr) {
			m_sizeCallback(m_window, width, height);
		}

		m_renderer->resize(width, height);
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