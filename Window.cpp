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

		for (int i = 0; i < m_framebuffers.size(); i++) m_framebuffers[i].~Framebuffer();
		m_renderPass.~RenderPass();
		m_swapchain.~Swapchain();
		m_surface.~Surface();
		glfwDestroyWindow(m_window);
	}

	void Window::init() {
		if (m_isInit) return;
		m_isInit = true;

		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
		m_window = glfwCreateWindow(m_width, m_height, m_title.c_str(), nullptr, nullptr);


		/*Surface*/ {
			m_surface.setGLFWwindow(m_window);
		}
		m_surface.init();

		/*Swapchain*/ {
			m_swapchain.setWidth(/*m_viewport.width*/1000);
			m_swapchain.setHeight(/*m_viewport.height*/600);
			m_swapchain.setPresentMode(VK_PRESENT_MODE_MAILBOX_KHR);
			m_swapchain.setSurface(m_surface);
		}
		m_swapchain.init();

		/*RenderPass*/ {
			VkAttachmentDescription colorAttachment;
			colorAttachment.flags = 0;
			colorAttachment.format = Zap::GlobalSettings::getColorFormat();
			colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
			colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
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

			VkSubpassDescription subpassDescription;
			subpassDescription.flags = 0;
			subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
			subpassDescription.inputAttachmentCount = 0;
			subpassDescription.pInputAttachments = nullptr;
			subpassDescription.colorAttachmentCount = 1;
			subpassDescription.pColorAttachments = pColorAttachmentReference;
			subpassDescription.pResolveAttachments = nullptr;
			subpassDescription.pDepthStencilAttachment = nullptr;
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

		/*Framebuffer*/ {
			m_framebuffers.resize(m_swapchain.getImageCount());
			for (int i = 0; i < m_swapchain.getImageCount(); i++) {
				m_framebuffers[i].setWidth(1000);
				m_framebuffers[i].setHeight(600);
				m_framebuffers[i].addAttachment(m_swapchain.getImageView(i));
				m_framebuffers[i].setRenderPass(m_renderPass);
				m_framebuffers[i].init();
			}
		}

	}

	bool Window::shouldClose() {
		return glfwWindowShouldClose(m_window);
	}

	void Window::show() {
		glfwShowWindow(m_window);
	}

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

	vk::RenderPass* Window::getRenderPass() {
		return &m_renderPass;
	}

	vk::Framebuffer* Window::getFramebuffer(uint32_t index) {
		return &m_framebuffers[index];
	}
	std::vector<vk::Framebuffer> Window::getFramebuffers() {
		return m_framebuffers;
	}

	void Window::pollEvents() {
		glfwPollEvents();
	}
}