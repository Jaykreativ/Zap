#include "Zap/Rendering/Renderer.h"
#include "VulkanUtils.h"

namespace Zap {
	Renderer::Renderer(Window& window)
		: m_window(window)
	{}

	Renderer::~Renderer() {}

	void Renderer::destroy() {
		for (RenderTemplate* renderTemplate : m_renderTemplates) {
			renderTemplate->destroy();
		}
		vk::destroyFence(m_renderComplete);
		vk::destroyFence(m_imageAvailable);
	}

	void Renderer::init() {
		/*Surface*/
		m_surface.setGLFWwindow(m_window.m_window);

		m_surface.init();

		/*Swapchain*/
		m_swapchain.setWidth(m_window.m_width);
		m_swapchain.setHeight(m_window.m_height);
		m_swapchain.setPresentMode(VK_PRESENT_MODE_MAILBOX_KHR);
		m_swapchain.setSurface(m_surface);
		m_swapchain.init();

		for (uint32_t i = 0; i < m_swapchain.getImageCount(); i++) {
			vk::changeImageLayout(m_swapchain.getImage(i), m_swapchain.getImageSubresourceRange(),
				VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
				VK_ACCESS_NONE, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT
			);
		}

		for (RenderTemplate* renderTemplate : m_renderTemplates) {
			renderTemplate->init();
		}

		m_commandBufferCount = m_swapchain.getImageCount();
		m_commandBuffers = new vk::CommandBuffer[m_commandBufferCount];
		for (uint32_t i = 0; i < m_commandBufferCount; i++) {
			m_commandBuffers[i].allocate();
		}

		recordCommandBuffers();

		vk::createFence(&m_renderComplete);
		vk::createFence(&m_imageAvailable);

		vk::acquireNextImage(m_swapchain, VK_NULL_HANDLE, m_imageAvailable, &m_currentImageIndex);
		vk::waitForFence(m_imageAvailable);
	}

	void Renderer::render() {
		if (glfwGetWindowAttrib(m_window.m_window, GLFW_ICONIFIED)) return;

		m_commandBuffers[m_currentImageIndex].submit(m_renderComplete);
		vk::waitForFence(m_renderComplete);

		vk::changeImageLayout(m_swapchain.getImage(m_currentImageIndex), m_swapchain.getImageSubresourceRange(),
			VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
			VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, VK_ACCESS_COLOR_ATTACHMENT_READ_BIT
		);

		vk::queuePresent(vkUtils::queueHandler::getQueue(), m_swapchain, m_currentImageIndex);

		vk::changeImageLayout(m_swapchain.getImage(m_currentImageIndex), m_swapchain.getImageSubresourceRange(),
			VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
			VK_ACCESS_COLOR_ATTACHMENT_READ_BIT, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT
		);

		vk::acquireNextImage(m_swapchain, VK_NULL_HANDLE, m_imageAvailable, &m_currentImageIndex);
		vk::waitForFence(m_imageAvailable);

	}

	void Renderer::addRenderTemplate(RenderTemplate* renderTemplate) {
		m_renderTemplates.push_back(renderTemplate);
	}

	void Renderer::recordCommandBuffers() {
		for (uint32_t i = 0; i < m_commandBufferCount; i++) {
			vk::CommandBuffer* cmd = &m_commandBuffers[i];
			cmd->begin(VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT);

			for (RenderTemplate* renderTemplate : m_renderTemplates) {
				renderTemplate->recordCommands(cmd, i);
			}

			cmd->end();
		}
	}
}