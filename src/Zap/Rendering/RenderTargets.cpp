#include "Zap/Rendering/RenderTargets.h"

#include "Zap/Rendering/Window.h"

namespace Zap {
	RenderTarget::RenderTarget()
	{}

	RenderTarget::~RenderTarget() {}

	bool RenderTarget::isValid() {
		return true;
	}

	// Image
	RenderTargetImage::RenderTargetImage()
		: RenderTarget(), Image()
	{
		setFormat(Zap::GlobalSettings::getColorFormat());
		setAspect(VK_IMAGE_ASPECT_COLOR_BIT);
		setUsage(VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);
		setLayout(VK_IMAGE_LAYOUT_PREINITIALIZED);
		setExtent({ 1, 1, 1 });
		
		init();
		allocate(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
		initView();
	}

	RenderTargetImage::~RenderTargetImage() {
		destroy();
	}

	// Window
	RenderTargetWindow::RenderTargetWindow(Window& window)
		: RenderTarget(), m_window(window)
	{}

	RenderTargetWindow::~RenderTargetWindow() {}
}
