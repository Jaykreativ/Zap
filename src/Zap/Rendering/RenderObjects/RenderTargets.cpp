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

	VkImageLayout RenderTarget::getInitialLayout() {
		return VK_IMAGE_LAYOUT_GENERAL;
	}

	VkImageLayout RenderTarget::getFinalLayout() {
		return VK_IMAGE_LAYOUT_GENERAL;
	}

	uint32_t RenderTarget::getImageCount() {
		return 1;
	}

	int RenderTarget::getImageIndex() {
		return -1;
	}

	// Image
	RenderTargetImage::RenderTargetImage(Renderer* pRenderer, VkFormat format, VkImageUsageFlags usage, VkMemoryPropertyFlags memoryProperties, VkImageLayout layout)
		: RenderTarget(pRenderer), m_image(format, pRenderer->getCommonTargetExtent(), usage, memoryProperties, layout)
	{}

	RenderTargetImage::~RenderTargetImage() {}

	void RenderTargetImage::recLayoutTransition(const vk::CommandBuffer& cmd, VkImageLayout oldLayout, VkImageLayout newLayout, VkAccessFlags srcAccessMask, VkAccessFlags dstAccessMask) {
		VkImageMemoryBarrier imageMemoryBarrier{ VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER, nullptr };
		imageMemoryBarrier.srcAccessMask = srcAccessMask;
		imageMemoryBarrier.dstAccessMask = dstAccessMask;
		imageMemoryBarrier.oldLayout = oldLayout;
		imageMemoryBarrier.newLayout = newLayout;
		imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		imageMemoryBarrier.image = m_image;
		imageMemoryBarrier.subresourceRange = m_image.getSubresourceRange();
		vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0, nullptr, 0, nullptr, 1, &imageMemoryBarrier);
	}

	VkExtent3D RenderTargetImage::getExtent() {
		return m_image.getExtent();
	}

	VkImageView RenderTargetImage::getImageView(uint32_t index) {
		return m_image;
	}

	void RenderTargetImage::resizeInternal(VkExtent2D extent) {
		m_image = Image2D(m_image, extent);
	}

	// Gui Image
	RenderTargetGuiImage::RenderTargetGuiImage(Renderer* pRenderer, VkFormat format, VkImageUsageFlags usage, VkMemoryPropertyFlags memoryProperties)
		: RenderTargetImage(pRenderer, format, usage, memoryProperties)
	{
		m_sampler.init();
		m_imageDescriptorSet = ImGui_ImplVulkan_AddTexture(m_sampler, getImageView(), VK_IMAGE_LAYOUT_GENERAL);
	}

	RenderTargetGuiImage::~RenderTargetGuiImage() {
		ImGui_ImplVulkan_RemoveTexture(m_imageDescriptorSet);
		m_sampler.destroy();
	}

	void RenderTargetGuiImage::resizeInternal(VkExtent2D size) {
		ImGui_ImplVulkan_RemoveTexture(m_imageDescriptorSet);
		RenderTargetImage::resizeInternal(size);
		m_imageDescriptorSet = ImGui_ImplVulkan_AddTexture(m_sampler, getImageView(), VK_IMAGE_LAYOUT_GENERAL);
	}

	// Window
	RenderTargetWindow::RenderTargetWindow(Renderer* pRenderer, Window& window)
		: RenderTarget(pRenderer), EventListener<WindowEvent::Resize>(window.getEventHandler()), m_window(window)
	{}

	RenderTargetWindow::~RenderTargetWindow() {}

	void RenderTargetWindow::recLayoutTransition(const vk::CommandBuffer& cmd, VkImageLayout oldLayout, VkImageLayout newLayout, VkAccessFlags srcAccessMask, VkAccessFlags dstAccessMask) {
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

	void RenderTargetWindow::resizeInternal(VkExtent2D extent) {}

	void RenderTargetWindow::callback(const WindowEvent::Resize& event) {
		resize({ event.width, event.height });
	}
}
