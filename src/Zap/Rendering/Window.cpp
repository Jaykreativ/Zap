#include "Zap/Rendering/Window.h"
#include "Zap/Rendering/Renderer.h"
#include "Zap/Rendering/PBRenderer.h"

namespace Zap {
	std::unordered_map<GLFWwindow*, Window*> Window::glfwWindowMap = {};

	Window::Window(uint32_t width, uint32_t height, std::string title)
		: m_width(width), m_height(height), m_title(title)
	{}

	Window::~Window() {
		if (!m_isInit) return;
		m_isInit = false;
		//TODO destroy renderer
		Window::glfwWindowMap.erase(m_window);
		glfwDestroyWindow(m_window);
	}

	void Window::resizeEventCallback(Zap::ResizeEvent& resizeEvent, void* data) {
		resizeEvent.pWindow->resize(resizeEvent.pWindow->m_window, resizeEvent.width, resizeEvent.height);
	}

	void Window::init() {
		if (m_isInit) return;
		m_isInit = true;


		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
		m_window = glfwCreateWindow(m_width, m_height, m_title.c_str(), nullptr, nullptr);

		glfwSetWindowSizeCallback(m_window, resizeGLFWCallback);
		glfwSetKeyCallback(m_window, keyGLFWCallback);
		glfwSetCursorPosCallback(m_window, cursorPosGLFWCallback);
		glfwSetMouseButtonCallback(m_window, mouseButtonGLFWCallback);
		glfwSetScrollCallback(m_window, scrollGLFWCallback);

		m_resizeEventHandler.addCallback(Window::resizeEventCallback);

		Window::glfwWindowMap[m_window] = this;
	}

	bool Window::shouldClose() {
		return glfwWindowShouldClose(m_window);
	}

	bool Window::isIconified() {
		return glfwGetWindowAttrib(m_window, GLFW_ICONIFIED);
	}

	void Window::show() {
		glfwShowWindow(m_window);
	}

	EventHandler<ResizeEvent>* Window::getResizeEventHandler() {
		return &m_resizeEventHandler;
	}

	EventHandler<KeyEvent>* Window::getKeyEventHandler() {
		return &m_keyEventHandler;
	}

	EventHandler<CursorPosEvent>* Window::getCursorPosEventHandler() {
		return &m_cursorPosEventHandler;
	}

	EventHandler<MouseButtonEvent>* Window::getMouseButtonEventHandler() {
		return &m_mouseButtonEventHandler;
	}

	EventHandler<ScrollEvent>* Window::getScrollEventHandler() {
		return &m_scrollEventHandler;
	}

	void Window::resizeGLFWCallback(GLFWwindow* window, int width, int height) {
		Window* pWindow = Window::glfwWindowMap.at(window);
		pWindow->getResizeEventHandler()->pushEvent(ResizeEvent(pWindow, width, height));
	}

	void Window::keyGLFWCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
		Window* pWindow = Window::glfwWindowMap.at(window);
		pWindow->getKeyEventHandler()->pushEvent(KeyEvent(pWindow, key, scancode, action, mods));
	}

	void Window::cursorPosGLFWCallback(GLFWwindow* window, double xPos, double yPos) {
		Window* pWindow = Window::glfwWindowMap.at(window);
		pWindow->getCursorPosEventHandler()->pushEvent(CursorPosEvent(pWindow, xPos, yPos));
	}

	void Window::mouseButtonGLFWCallback(GLFWwindow* window, int button, int action, int mods) {
		Window* pWindow = Window::glfwWindowMap.at(window);
		pWindow->getMouseButtonEventHandler()->pushEvent(MouseButtonEvent(pWindow, button, action, mods));
	}

	void Window::scrollGLFWCallback(GLFWwindow* window, double xoffset, double yoffset) {
		Window* pWindow = Window::glfwWindowMap.at(window);
		pWindow->getScrollEventHandler()->pushEvent(ScrollEvent(pWindow, xoffset, yoffset));
	}

	void Window::resize(GLFWwindow* window, int width, int height) {
		m_width = width;
		m_height = height;

		if (m_height <= 0) return;

		vk::deviceWaitIdle();

		if (m_sizeCallback != nullptr) {
			m_sizeCallback(m_window, width, height);
		}

		m_renderer->onWindowResize(width, height);
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