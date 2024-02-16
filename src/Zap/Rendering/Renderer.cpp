#include "Zap/Rendering/Renderer.h"
#include "VulkanUtils.h"

namespace Zap {
	Renderer::Renderer(Window& window)
		: m_window(window)
	{}

	Renderer::~Renderer() {}

	void Renderer::destroy() {
		for (uint32_t i = 0; i < m_commandBufferCount; i++) {
			m_commandBuffers[i].free();
		}
		for (RenderTemplate* renderTemplate : m_renderTemplates) {
			renderTemplate->destroy();
		}
		m_swapchain.~Swapchain();
		m_surface.~Surface();
		vk::destroyFence(m_renderComplete);
		vk::destroyFence(m_imageAvailable);
	}

	void Renderer::init() {
		m_window.m_renderer = this;

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
			renderTemplate->onRendererInit();
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
		for (RenderTemplate* renderTemplate : m_renderTemplates) {
			renderTemplate->beforeRender();
		}

		// Render
		m_commandBuffers[m_currentImageIndex].submit(m_renderComplete);
		vk::waitForFence(m_renderComplete); // TODO use semaphores for parallelization

		for (RenderTemplate* renderTemplate : m_renderTemplates) {
			renderTemplate->afterRender();
		}

		if (glfwGetWindowAttrib(m_window.m_window, GLFW_ICONIFIED)) return;

		// Present swapchain image
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

	void Renderer::update() {
		recordCommandBuffers();
	}

	void Renderer::beginRecord() {
		m_recordedFunctions.clear();
		m_recordedParams.clear();
	}

	void Renderer::endRecord() {
		recordCommandBuffers();
	}

	void Renderer::recRenderTemplate(RenderTemplate* renderTemplate) {
		m_recordedFunctions.push_back(eRENDER_TEMPLATE);
		m_recordedParams.resize(m_recordedParams.size()+sizeof(RenderTemplate*));
		memcpy(&m_recordedParams[m_recordedParams.size()-sizeof(RenderTemplate*)], &renderTemplate, sizeof(RenderTemplate*));
	}

	void Renderer::recChangeImageLayout(Image* pImage, VkImageLayout layout, VkAccessFlags accessMask) {
		m_recordedFunctions.push_back(eCHANGE_IMAGE_LAYOUT);
		uint32_t oldSize = m_recordedParams.size();
		m_recordedParams.resize(m_recordedParams.size() + sizeof(Image*) + sizeof(VkImageLayout) + sizeof(VkAccessFlags));
		memcpy(&m_recordedParams[oldSize], &pImage, sizeof(Image*));
		memcpy(&m_recordedParams[oldSize+sizeof(Image*)], &layout, sizeof(VkImageLayout));
		memcpy(&m_recordedParams[oldSize+sizeof(Image*)+sizeof(VkImageLayout)], &accessMask, sizeof(VkAccessFlags));
	}

	void Renderer::addRenderTemplate(RenderTemplate* renderTemplate) {
		m_renderTemplates.push_back(renderTemplate);
	}

	void Renderer::recordCommandBuffers() {
		if (!m_recordedFunctions.size()) return;

		for (uint32_t i = 0; i < m_commandBufferCount; i++) {
			vk::CommandBuffer* cmd = &m_commandBuffers[i];
			cmd->begin(VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT);

			char* currentParam = &m_recordedParams[0];
			for (FunctionType function : m_recordedFunctions) {
				switch (function)
				{
				case eRENDER_TEMPLATE:
					RenderTemplate* pRenderTemplate;
					memcpy(&pRenderTemplate, currentParam, sizeof(RenderTemplate*));
					currentParam += sizeof(RenderTemplate*);
					pRenderTemplate->recordCommands(cmd, i);
					break;
				case eCHANGE_IMAGE_LAYOUT:
					Image* pImage;
					memcpy(&pImage, currentParam, sizeof(Image*));
					currentParam += sizeof(Image*);

					VkImageLayout layout;
					memcpy(&layout, currentParam, sizeof(VkImageLayout));
					currentParam += sizeof(VkImageLayout);

					VkAccessFlags accessMask;
					memcpy(&accessMask, currentParam, sizeof(VkAccessFlags));
					currentParam += sizeof(VkAccessFlags);
					pImage->cmdChangeLayout(*cmd, layout, accessMask);
					break;
				default:
					ZP_ASSERT(false, "Unknown function type");
					break;
				}
			}

			cmd->end();
		}
	}

	void Renderer::onWindowResize(int width, int height) {
		m_swapchain.setWidth(width);
		m_swapchain.setHeight(height);
		m_swapchain.update();

		for (uint32_t i = 0; i < m_swapchain.getImageCount(); i++) {
			vk::changeImageLayout(m_swapchain.getImage(i), m_swapchain.getImageSubresourceRange(),
				VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
				VK_ACCESS_NONE, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT
			);
		}
		
		for (RenderTemplate* renderTemplate : m_renderTemplates) renderTemplate->onWindowResize(width, height);

		recordCommandBuffers();
		
		vk::acquireNextImage(m_swapchain, VK_NULL_HANDLE, m_imageAvailable, &m_currentImageIndex);
		vk::waitForFence(m_imageAvailable);
	}
}