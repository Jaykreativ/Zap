#include "Zap/Rendering/Window.h"
#include "Zap/Rendering/Renderer.h"
#include "Zap/Rendering/PBRenderer.h"
#include "VulkanUtils.h"

void resizeCallback(GLFWwindow* window, int width, int height) {
	for (uint32_t i = 0; i < Zap::objects::windows.size(); i++) {
		if (Zap::objects::windows[i]->getGLFWwindow() == window) {
			Zap::objects::windows[i]->resizeVk(window, width, height);
		}
	}
}

namespace Zap {
	Window::Window(uint32_t width, uint32_t height, std::string title)
		: m_width(width), m_height(height), m_title(title)
	{}

	Window::~Window() {
		if (!m_isInit) return;
		m_isInit = false;

		vk::destroyFence(m_imageAvailable);
		for (uint32_t i = 0; i < m_swapchain.getImageCount(); i++) m_clearCommandBuffers[i].free();
		m_clearDepthStencilCommandBuffer.free();
		for (int i = 0; i < m_framebuffers.size(); i++) m_framebuffers[i].~Framebuffer();
		m_renderPass.~RenderPass();
		m_depthImage.~Image();
		m_swapchain.~Swapchain();
		m_surface.~Surface();
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

		/*Surface*/
		m_surface.setGLFWwindow(m_window);
		
		m_surface.init();

		/*Swapchain*/
		m_swapchain.setWidth(m_width);
		m_swapchain.setHeight(m_height);
		m_swapchain.setPresentMode(VK_PRESENT_MODE_MAILBOX_KHR);
		m_swapchain.setSurface(m_surface);
		m_swapchain.init();

		for (uint32_t i = 0; i < m_swapchain.getImageCount(); i++) {
			vk::changeImageLayout(m_swapchain.getImage(i), m_swapchain.getImageSubresourceRange(),
				VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
				VK_ACCESS_NONE, VK_ACCESS_COLOR_ATTACHMENT_READ_BIT
			);
		}

		/*Depth Image*/
		m_depthImage = vk::Image();
		m_depthImage.setAspect(VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT);
		m_depthImage.setExtent({ m_width, m_height, 1 });
		m_depthImage.setFormat(Zap::GlobalSettings::getDepthStencilFormat());
		m_depthImage.setInitialLayout(VK_IMAGE_LAYOUT_UNDEFINED);
		m_depthImage.setType(VK_IMAGE_TYPE_2D);
		m_depthImage.setUsage(VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT);
		m_depthImage.setMemoryProperties(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
		m_depthImage.init();
		m_depthImage.allocate(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
		m_depthImage.initView();

		m_depthImage.changeLayout(VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, 0, VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT);

		/*RenderPass*/ {
			VkAttachmentDescription colorAttachment;// Color Attachment
			colorAttachment.flags = 0;
			colorAttachment.format = Zap::GlobalSettings::getColorFormat();
			colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
			colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
			colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
			colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			colorAttachment.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
			colorAttachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

			m_renderPass.addAttachmentDescription(colorAttachment);

			VkAttachmentReference* pColorAttachmentReference;
			{
				VkAttachmentReference tmp;
				tmp.attachment = 0;
				tmp.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
				pColorAttachmentReference = &tmp;

				m_renderPass.addAttachmentReference(&pColorAttachmentReference);
			}

			VkAttachmentDescription depthStencilAttachment;// Depth Stencil Attachment
			depthStencilAttachment.flags = 0;
			depthStencilAttachment.format = Zap::GlobalSettings::getDepthStencilFormat();
			depthStencilAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
			depthStencilAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
			depthStencilAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
			depthStencilAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			depthStencilAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			depthStencilAttachment.initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
			depthStencilAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

			m_renderPass.addAttachmentDescription(depthStencilAttachment);

			VkAttachmentReference* pDepthStencilAttachmentReference;
			{
				VkAttachmentReference tmp;
				tmp.attachment = 1;
				tmp.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
				pDepthStencilAttachmentReference = &tmp;

				m_renderPass.addAttachmentReference(&pDepthStencilAttachmentReference);
			}

			VkSubpassDescription subpassDescription;
			subpassDescription.flags = 0;
			subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
			subpassDescription.inputAttachmentCount = 0;
			subpassDescription.pInputAttachments = nullptr;
			subpassDescription.colorAttachmentCount = 1;
			subpassDescription.pColorAttachments = pColorAttachmentReference;
			subpassDescription.pResolveAttachments = nullptr;
			subpassDescription.pDepthStencilAttachment = pDepthStencilAttachmentReference;
			subpassDescription.preserveAttachmentCount = 0;
			subpassDescription.pPreserveAttachments = nullptr;

			m_renderPass.addSubpassDescription(subpassDescription);

			VkSubpassDependency subpassDependency;
			subpassDependency.srcSubpass = VK_SUBPASS_EXTERNAL;
			subpassDependency.dstSubpass = 0;
			subpassDependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			subpassDependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			subpassDependency.srcAccessMask = 0;
			subpassDependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
			subpassDependency.dependencyFlags = 0;

			m_renderPass.addSubpassDependency(subpassDependency);
		}
		m_renderPass.init();

		/*Framebuffer*/
		m_framebuffers.resize(m_swapchain.getImageCount());
		for (int i = 0; i < m_swapchain.getImageCount(); i++) {
			m_framebuffers[i].setWidth(m_width);
			m_framebuffers[i].setHeight(m_height);
			m_framebuffers[i].addAttachment(m_swapchain.getImageView(i));
			m_framebuffers[i].addAttachment(m_depthImage.getVkImageView());
			m_framebuffers[i].setRenderPass(m_renderPass);
			m_framebuffers[i].init();
		}

		vk::createFence(&m_imageAvailable);

		/*CommandBuffers*/
		m_clearCommandBuffers = new vk::CommandBuffer[m_swapchain.getImageCount()];
		for (uint32_t i = 0; i < m_swapchain.getImageCount(); i++) {
			m_clearCommandBuffers[i].allocate();
		}
		recordClearCommandBuffers();
		
		m_clearDepthStencilCommandBuffer.allocate();
		recordClearDepthStencilCommandBuffer();

		vk::acquireNextImage(m_swapchain, VK_NULL_HANDLE, m_imageAvailable, &m_currentImageIndex);
		vk::waitForFence(m_imageAvailable);
	}

	void Window::clear() {
		clearColor();
		clearDepthStencil();
	}

	void Window::clearColor() {
		m_clearCommandBuffers[m_currentImageIndex].submit();
	}

	void Window::clearDepthStencil() {
		m_clearDepthStencilCommandBuffer.submit();
	}

	void Window::swapBuffers() {
		if (glfwGetWindowAttrib(m_window, GLFW_ICONIFIED)) return;
		if (m_height <= 0) return;

		vk::changeImageLayout(m_swapchain.getImage(m_currentImageIndex), m_swapchain.getImageSubresourceRange(),
			VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
			VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, VK_ACCESS_COLOR_ATTACHMENT_READ_BIT
		);

		vk::queuePresent(vkUtils::queueHandler::getQueue(), m_swapchain, m_currentImageIndex);

		vk::acquireNextImage(m_swapchain, VK_NULL_HANDLE, m_imageAvailable, &m_currentImageIndex);
		vk::waitForFence(m_imageAvailable);
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

	void Window::recordClearCommandBuffers() {
		for (uint32_t i = 0; i < m_swapchain.getImageCount(); i++) {
			vk::CommandBuffer& cmd = m_clearCommandBuffers[i];
			cmd.begin(VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT);

			VkImageMemoryBarrier imageMemoryBarrier;
			imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
			imageMemoryBarrier.pNext = nullptr;
			imageMemoryBarrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
			imageMemoryBarrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
			imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
			imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			imageMemoryBarrier.image = m_swapchain.getImage(i);
			imageMemoryBarrier.subresourceRange = m_swapchain.getImageSubresourceRange();

			vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0, nullptr, 0, nullptr, 1, &imageMemoryBarrier);

			VkClearColorValue clearColor = { 0.1, 0.1, 0.1, 1 };
			vkCmdClearColorImage(cmd, m_swapchain.getImage(i), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, &clearColor, 1, &m_swapchain.getImageSubresourceRange());

			imageMemoryBarrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
			imageMemoryBarrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
			imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

			vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0, nullptr, 0, nullptr, 1, &imageMemoryBarrier);

			cmd.end();
		}
	}

	void Window::recordClearDepthStencilCommandBuffer() {
		vk::CommandBuffer& cmd = m_clearDepthStencilCommandBuffer;
		cmd.begin(VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT);

		m_depthImage.cmdChangeLayout(
			cmd,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
			VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT
		);


		VkClearDepthStencilValue clearDepthStencil = { 1, 0 };
		vkCmdClearDepthStencilImage(
			cmd,
			m_depthImage,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			&clearDepthStencil,
			1, &m_depthImage.getSubresourceRange()
		);

		m_depthImage.cmdChangeLayout(
			cmd,
			VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
			VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
			VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT
		);

		cmd.end();
	}

	void Window::resizeVk(GLFWwindow* window, int width, int height) {
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
		vk::waitForFence(m_imageAvailable);
	}

	void Window::addRenderer(Renderer* renderer) {
		m_renderers.push_back(renderer);
	}

	/*Getter*/
	uint32_t Window::getWidth() {
		return m_width;
	}
	uint32_t Window::getHeight() {
		return m_height;
	}

	GLFWwindow* Window::getGLFWwindow() {
		return m_window;
	}

	vk::Surface* Window::getSurface() {
		return &m_surface;
	}

	vk::Swapchain* Window::getSwapchain() {
		return &m_swapchain;
	}

	vk::Image* Window::getDepthImage() {
		return &m_depthImage;
	}

	vk::RenderPass* Window::getRenderPass() {
		return &m_renderPass;
	}

	vk::Framebuffer* Window::getFramebuffer(uint32_t index) {
		return &m_framebuffers[index];
	}
	std::vector<vk::Framebuffer> Window::getFramebuffers() {
		return m_framebuffers;
	}
	vk::Framebuffer* Window::getFramebufferPtr() {
		return m_framebuffers.data();
	}

	uint32_t Window::getCurrentImageIndex() {
		return m_currentImageIndex;
	}

	VkFence Window::getImageAvailableFence() {
		return m_imageAvailable;
	}

	Renderer* Window::getRenderer(uint32_t index) {
		return m_renderers[index];
	}

	void Window::pollEvents() {
		glfwPollEvents();
	}
}