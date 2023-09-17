#include "Window.h"
#include "VulkanUtils.h"

namespace Zap {
	Window::Window(uint32_t width, uint32_t height, std::string title) {
		m_width = width;
		m_height = height;
		m_title = title;
	}

	Window::~Window() {
		if (!m_isInit) return;
		m_isInit = false;

		vk::destroyFence(m_imageAvailable);
		for (int i = 0; i < m_framebuffers.size(); i++) m_framebuffers[i].~Framebuffer();
		m_renderPass.~RenderPass();
		for (int i = 0; i < m_depthImages.size(); i++) m_depthImages[i].~Image();
		m_swapchain.~Swapchain();
		m_surface.~Surface();
		glfwDestroyWindow(m_window);
	}

	void Window::init() {
		if (m_isInit) return;
		m_isInit = true;

		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
		glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
		m_window = glfwCreateWindow(m_width, m_height, m_title.c_str(), nullptr, nullptr);

		/*Surface*/
		m_surface.setGLFWwindow(m_window);
		
		m_surface.init();

		/*Swapchain*/
		m_swapchain.setWidth(m_width);
		m_swapchain.setHeight(m_height);
		m_swapchain.setPresentMode(VK_PRESENT_MODE_MAILBOX_KHR);
		m_swapchain.setSurface(m_surface);
		m_swapchain.init();

		/*Depth Image*/
		m_depthImages.resize(m_swapchain.getImageCount());
		for (int i = 0; i < m_swapchain.getImageCount(); i++) {
			m_depthImages[i] = vk::Image();
			m_depthImages[i].setAspect(VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT);
			m_depthImages[i].setExtent({ m_width, m_height, 1 });
			m_depthImages[i].setFormat(Zap::GlobalSettings::getDepthStencilFormat());
			m_depthImages[i].setInitialLayout(VK_IMAGE_LAYOUT_UNDEFINED);
			m_depthImages[i].setType(VK_IMAGE_TYPE_2D);
			m_depthImages[i].setUsage(VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT);
			m_depthImages[i].init();
			m_depthImages[i].allocate(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
			m_depthImages[i].initView();
			m_depthImages[i].changeLayout(
				VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, 
				0, 
				VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT
			);
		}

		/*RenderPass*/ {
			VkAttachmentDescription colorAttachment;// Color Attachment
			colorAttachment.flags = 0;
			colorAttachment.format = Zap::GlobalSettings::getColorFormat();
			colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
			colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
			colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

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
			depthStencilAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			depthStencilAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
			depthStencilAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			depthStencilAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			depthStencilAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
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
			m_framebuffers[i].addAttachment(m_depthImages[i].getVkImageView());
			m_framebuffers[i].setRenderPass(m_renderPass);
			m_framebuffers[i].init();
		}

		vk::createFence(&m_imageAvailable);

		vk::acquireNextImage(m_swapchain, VK_NULL_HANDLE, m_imageAvailable, &m_currentImageIndex);
		vk::waitForFence(m_imageAvailable);
	}

	void Window::swapBuffers() {
		if (glfwGetWindowAttrib(m_window, GLFW_ICONIFIED)) return;
		vk::queuePresent(vkUtils::queueHandler::getQueue(), m_swapchain, m_currentImageIndex);

		vk::acquireNextImage(m_swapchain, VK_NULL_HANDLE, m_imageAvailable, &m_currentImageIndex);
		vk::waitForFence(m_imageAvailable);
	}

	bool Window::shouldClose() {
		return glfwWindowShouldClose(m_window);
	}

	void Window::setKeyCallback(GLFWkeyfun callback) {
		glfwSetKeyCallback(m_window, callback);
	}

	void Window::show() {
		glfwShowWindow(m_window);
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

	std::vector<vk::Image>* Window::getDepthImages() {
		return &m_depthImages;
	}

	vk::Image* Window::getDepthImage(int index) {
		return &m_depthImages[index];
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

	void Window::pollEvents() {
		glfwPollEvents();
	}
}