#include "Window.h"

namespace Zap {
	Window::Window(uint32_t width, uint32_t height, std::string title) {
		m_width = width;
		m_height = height;
		m_title = title;
	}

	Window::~Window() {
		if (!m_isInit) return;
		m_isInit = false;

		glfwDestroyWindow(m_window);
	}

	void Window::init() {
		if (m_isInit) return;
		m_isInit = true;

		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
		m_window = glfwCreateWindow(m_width, m_height, m_title.c_str(), nullptr, nullptr);
	}

	bool Window::shouldClose() {
		return glfwWindowShouldClose(m_window);
	}

	void Window::show() {
		glfwShowWindow(m_window);
	}

	void Window::pollEvents() {
		glfwPollEvents();
	}
}