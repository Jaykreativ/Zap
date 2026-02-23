#include "Zap/Rendering/Window.h"
#include "Zap/Rendering/Renderer.h"
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

		vk::CommandBuffer cmdBuffer = vk::CommandBuffer(true);
		cmdBuffer.begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

		VkImageMemoryBarrier imageMemoryBarrier{ VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER, nullptr };
		imageMemoryBarrier.srcAccessMask = VK_ACCESS_NONE;
		imageMemoryBarrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
		imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL; // TODO get the layout from a windowRenderTarget
		imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
		imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		imageMemoryBarrier.image = m_swapchain.getImage(m_currentSwapchainImageIndex)->getVkImage();
		imageMemoryBarrier.subresourceRange = *m_swapchain.getImage(m_currentSwapchainImageIndex)->getSubresourceRange();
		vkCmdPipelineBarrier(cmdBuffer, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0, nullptr, 0, nullptr, 1, &imageMemoryBarrier);

		cmdBuffer.end();
		cmdBuffer.submit(); cmdBuffer.free();

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

	WindowEventHandler& Window::getEventHandler() {
		return m_eventHandler;
	}

	void Window::resizeGLFWCallback(GLFWwindow* window, int width, int height) {
		Window* pWindow = Window::glfwWindowMap.at(window);
		pWindow->resize(window, width, height);
		pWindow->getEventHandler().EventHandler<WindowEvent::Resize>::pushEvent(WindowEvent::Resize(pWindow, width, height));
	}

	void Window::keyGLFWCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
		Window* pWindow = Window::glfwWindowMap.at(window);
		pWindow->getEventHandler().EventHandler<WindowEvent::Key>::pushEvent(WindowEvent::Key(pWindow, key, scancode, action, mods));
	}

	void Window::cursorPosGLFWCallback(GLFWwindow* window, double xPos, double yPos) {
		Window* pWindow = Window::glfwWindowMap.at(window);
		pWindow->getEventHandler().EventHandler<WindowEvent::CursorPos>::pushEvent(WindowEvent::CursorPos(pWindow, xPos, yPos));
	}

	void Window::mouseButtonGLFWCallback(GLFWwindow* window, int button, int action, int mods) {
		Window* pWindow = Window::glfwWindowMap.at(window);
		pWindow->getEventHandler().EventHandler<WindowEvent::MouseButton>::pushEvent(WindowEvent::MouseButton(pWindow, button, action, mods));
	}

	void Window::scrollGLFWCallback(GLFWwindow* window, double xoffset, double yoffset) {
		Window* pWindow = Window::glfwWindowMap.at(window);
		pWindow->getEventHandler().EventHandler<WindowEvent::Scroll>::pushEvent(WindowEvent::Scroll(pWindow, xoffset, yoffset));
	}

	void Window::dragDropGLFWCallback(GLFWwindow* window, int path_count, const char* paths[]) {
		Window* pWindow = Window::glfwWindowMap.at(window);
		pWindow->getEventHandler().EventHandler<WindowEvent::DragDrop>::pushEvent(WindowEvent::DragDrop(pWindow, path_count, paths));
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