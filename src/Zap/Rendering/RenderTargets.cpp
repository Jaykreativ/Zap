#include "Zap/Rendering/RenderTargets.h"

#include "Zap/Rendering/Window.h"
#include "Zap/Rendering/Renderer.h"

namespace Zap {
	void RenderTarget::resize(glm::vec2 size) {
		m_pRenderer->resize(size);
	}

	bool RenderTarget::isValid() {
		return true;
	}

	uint32_t RenderTarget::getImageCount() {
		return 1;
	}

	int RenderTarget::getImageIndex() {
		return -1;
	}

	// Image
	RenderTargetImage::RenderTargetImage()
		: RenderTarget()
	{
		m_image.setFormat(Zap::GlobalSettings::getColorFormat());
		m_image.setAspect(VK_IMAGE_ASPECT_COLOR_BIT);
		m_image.setUsage(VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);
		m_image.setLayout(VK_IMAGE_LAYOUT_PREINITIALIZED);
		m_image.setExtent({ 1, 1, 1 });

		m_image.init();
		m_image.allocate(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
		m_image.initView();
	}

	RenderTargetImage::~RenderTargetImage() {
		m_image.destroy();
	}

	VkExtent3D RenderTargetImage::getExtent() {
		return m_image.getExtent();
	}

	VkImageView RenderTargetImage::getImageView(uint32_t index) {
		return m_image.getVkImageView();
	}

	void RenderTargetImage::resizeInternal(glm::vec2 size) {
		m_image.resize(size.x, size.y);
	}

	// Window
	RenderTargetWindow::RenderTargetWindow(Window& window)
		: RenderTarget(), m_window(window)
	{
		m_window.getResizeEventHandler()->addCallback(resizeCallback, this);
	}

	RenderTargetWindow::~RenderTargetWindow() {
		m_window.getResizeEventHandler()->removeCallback(resizeCallback, this);
	}

	VkExtent3D RenderTargetWindow::getExtent() {
		return { m_window.getWidth(), m_window.getHeight() , 1};
	}

	uint32_t RenderTargetWindow::getImageCount() {
		return m_window.getSwapchain()->getImageCount();
	}

	int RenderTargetWindow::getImageIndex() {
		return m_window.getSwapchainImageIndex();
	}

	VkImageView RenderTargetWindow::getImageView(uint32_t index) {
		return m_window.getSwapchain()->getImage(index)->getVkImageView();
	}

	void RenderTargetWindow::resizeInternal(glm::vec2 size) {}

	void RenderTargetWindow::resizeCallback(ResizeEvent& eventParams, void* customParams) {
		RenderTargetWindow* pObj = reinterpret_cast<RenderTargetWindow*>(customParams);
		pObj->resize({ eventParams.width, eventParams.height });
	}
}
