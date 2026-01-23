#include "Zap/Rendering/RenderObjects/RenderTargets.h"

#include "Zap/Rendering/Window.h"
#include "Zap/Rendering/Renderer.h"

#include "backends/imgui_impl_vulkan.h"
#include "imgui.h"

namespace Zap {
	RenderTarget::RenderTarget(Renderer* pRenderer)
		: RenderObject(pRenderer)
	{}

	void RenderTarget::resize(glm::vec2 size) {
		m_pRenderer->resize(size);
	}

	bool RenderTarget::isValid() {
		return true;
	}

	void RenderTarget::setInitialLayout(VkImageLayout initialLayout) {
		m_initialLayout = initialLayout;
	}

	void RenderTarget::setFinalLayout(VkImageLayout finalLayout) {
		m_finalLayout = finalLayout;
	}

	VkImageLayout RenderTarget::getInitialLayout() {
		return m_initialLayout;
	}

	VkImageLayout RenderTarget::getFinalLayout() {
		return m_finalLayout;
	}

	uint32_t RenderTarget::getImageCount() {
		return 1;
	}

	int RenderTarget::getImageIndex() {
		return -1;
	}

	// RenderTargetDescriptorSet
	
	// Image
	RenderTargetImage::RenderTargetImage(Renderer* pRenderer)
		: RenderTarget(pRenderer)
	{}

	RenderTargetImage::~RenderTargetImage() {
		m_image.destroy();
	}

	void RenderTargetImage::recLayoutTransition(vk::CommandBuffer& cmd, VkImageLayout oldLayout, VkImageLayout newLayout, VkAccessFlags srcAccessMask, VkAccessFlags dstAccessMask) {
		VkImageMemoryBarrier imageMemoryBarrier{ VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER, nullptr };
		imageMemoryBarrier.srcAccessMask = srcAccessMask;
		imageMemoryBarrier.dstAccessMask = dstAccessMask;
		imageMemoryBarrier.oldLayout = oldLayout;
		imageMemoryBarrier.newLayout = newLayout;
		imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		imageMemoryBarrier.image = m_image;
		imageMemoryBarrier.subresourceRange = *m_image.getSubresourceRange();
		vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0, nullptr, 0, nullptr, 1, &imageMemoryBarrier);
	}

	void RenderTargetImage::setAspect(VkImageAspectFlags aspect) {
		m_image.setAspect(aspect);
	}

	void RenderTargetImage::setFormat(VkFormat format) {
		m_image.setFormat(format);
	}

	void RenderTargetImage::setUsage(VkImageUsageFlags usage) {
		m_image.setUsage(usage);
	}

	void RenderTargetImage::init(VkMemoryPropertyFlags memoryProperty) {
		m_image.init();
		m_image.allocate(memoryProperty);
		m_image.initView();
		if (getInitialLayout() != VK_IMAGE_LAYOUT_UNDEFINED)
			m_image.changeLayout(getInitialLayout(), VK_ACCESS_MEMORY_WRITE_BIT);
	}

	Image& RenderTargetImage::getImage() {
		return m_image;
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

	// Gui Image
	RenderTargetGuiImage::RenderTargetGuiImage(Renderer* pRenderer)
		: RenderTargetImage(pRenderer)
	{
		m_sampler.init();
		setInitialLayout(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
		setFinalLayout(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
	}

	RenderTargetGuiImage::~RenderTargetGuiImage() {
		ImGui_ImplVulkan_RemoveTexture(m_imageDescriptorSet);
		m_sampler.destroy();
	}

	void RenderTargetGuiImage::init(VkMemoryPropertyFlags memoryProperty) {
		RenderTargetImage::init(memoryProperty);
		m_imageDescriptorSet = ImGui_ImplVulkan_AddTexture(m_sampler, getImageView(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
	}

	void RenderTargetGuiImage::resizeInternal(glm::vec2 size) {
		ImGui_ImplVulkan_RemoveTexture(m_imageDescriptorSet);
		RenderTargetImage::resizeInternal(size);
		m_imageDescriptorSet = ImGui_ImplVulkan_AddTexture(m_sampler, getImageView(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
	}

	// Window
	RenderTargetWindow::RenderTargetWindow(Renderer* pRenderer, Window& window)
		: RenderTarget(pRenderer), m_window(window)
	{
		m_window.getResizeEventHandler()->addCallback(resizeCallback, this);
	}

	RenderTargetWindow::~RenderTargetWindow() {
		m_window.getResizeEventHandler()->removeCallback(resizeCallback, this);
	}

	void RenderTargetWindow::recLayoutTransition(vk::CommandBuffer& cmd, VkImageLayout oldLayout, VkImageLayout newLayout, VkAccessFlags srcAccessMask, VkAccessFlags dstAccessMask) {
		auto image = m_window.getSwapchain()->getImage(m_window.getSwapchainImageIndex());

		VkImageMemoryBarrier imageMemoryBarrier{ VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER, nullptr };
		imageMemoryBarrier.srcAccessMask = srcAccessMask;
		imageMemoryBarrier.dstAccessMask = dstAccessMask;
		imageMemoryBarrier.oldLayout = oldLayout;
		imageMemoryBarrier.newLayout = newLayout;
		imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		imageMemoryBarrier.image = image->getVkImage();
		imageMemoryBarrier.subresourceRange = *image->getSubresourceRange();
		vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0, nullptr, 0, nullptr, 1, &imageMemoryBarrier);
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
