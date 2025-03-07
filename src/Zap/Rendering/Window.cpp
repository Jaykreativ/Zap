#include "Zap/Rendering/Window.h"
#include "Zap/Rendering/Renderer.h"
#include "Zap/Rendering/PBRenderer.h"
#include "VulkanUtils.h"

namespace Zap {
	std::unordered_map<GLFWwindow*, Window*> Window::glfwWindowMap = {};

	Window::Window(uint32_t width, uint32_t height, std::string title)
		: m_width(width), m_height(height), m_title(title)
	{}

	Window::~Window() {
		destroy();
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
		glfwSetDropCallback(m_window, dragDropGLFWCallback);

		m_surface.setGLFWwindow(m_window);
		m_surface.init();

		m_swapchain.setWidth(m_width);
		m_swapchain.setHeight(m_height);
		m_swapchain.setPresentMode(VK_PRESENT_MODE_MAILBOX_KHR);
		m_swapchain.setSurface(m_surface);
		m_swapchain.init();

		vk::createFence(&m_imageAvailable);

		vk::acquireNextImage(m_swapchain, VK_NULL_HANDLE, m_imageAvailable, &m_currentSwapchainImageIndex);
		vk::waitForFence(m_imageAvailable);

		Window::glfwWindowMap[m_window] = this;
	}

	void Window::destroy() {
		if (!m_isInit) return;
		m_isInit = false;
		//TODO destroy renderer
		Window::glfwWindowMap.erase(m_window);
		vk::destroyFence(m_imageAvailable);
		m_swapchain.destroy();
		m_surface.destroy();
		glfwDestroyWindow(m_window);
	}

	void Window::present() {
		if (glfwGetWindowAttrib(m_window, GLFW_ICONIFIED)) return;

		m_swapchain.getImage(m_currentSwapchainImageIndex)->changeLayout(VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, VK_ACCESS_COLOR_ATTACHMENT_READ_BIT);

		vk::queuePresent(vkUtils::queueHandler::getQueue(), m_swapchain, m_currentSwapchainImageIndex);

		vk::acquireNextImage(m_swapchain, VK_NULL_HANDLE, m_imageAvailable, &m_currentSwapchainImageIndex);
		vk::waitForFence(m_imageAvailable);
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

	EventHandler<DragDropEvent>* Window::getDragDropEventHandler() {
		return &m_dragDropEventHandler;
	}

	void Window::resizeGLFWCallback(GLFWwindow* window, int width, int height) {
		Window* pWindow = Window::glfwWindowMap.at(window);
		pWindow->resize(window, width, height);
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

	void Window::dragDropGLFWCallback(GLFWwindow* window, int path_count, const char* paths[]) {
		Window* pWindow = Window::glfwWindowMap.at(window);
		pWindow->getDragDropEventHandler()->pushEvent(DragDropEvent(pWindow, path_count, paths));
	}

	void Window::resize(GLFWwindow* window, int width, int height) {
		m_width = width;
		m_height = height;

		if (m_height <= 0) return;

		vk::deviceWaitIdle();

		if (m_sizeCallback != nullptr) {
			m_sizeCallback(m_window, width, height);
		}

		m_swapchain.setWidth(width);
		m_swapchain.setHeight(height);
		m_swapchain.update();

		vk::acquireNextImage(m_swapchain, VK_NULL_HANDLE, m_imageAvailable, &m_currentSwapchainImageIndex);
		vk::waitForFence(m_imageAvailable);
	}

	/*Getter*/
	uint32_t Window::getWidth() {
		return m_width;
	}
	uint32_t Window::getHeight() {
		return m_height;
	}

	uint32_t Window::getSwapchainImageIndex() {
		return m_currentSwapchainImageIndex;
	}

	const vk::Surface* Window::getSurface() {
		return &m_surface;
	}

	const vk::Swapchain* Window::getSwapchain() {
		return &m_swapchain;
	}

	void Window::pollEvents() {
		glfwPollEvents();
	}
}