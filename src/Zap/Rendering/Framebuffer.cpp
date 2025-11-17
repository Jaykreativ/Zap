#include "Zap/Rendering/Framebuffer.h"

#include "Zap/Rendering/Renderer.h"

namespace Zap {
	Framebuffer* FramebufferHandle::get() {
		return m_renderer->getFramebuffer(m_handle);
	}
	const Framebuffer* FramebufferHandle::get() const {
		return m_renderer->getFramebuffer(m_handle);
	}

	Framebuffer* FramebufferHandle::operator->() {
		return get();
	}

	FramebufferHandle::operator bool() const {
		return m_renderer != nullptr && get() != nullptr;
	}

	Framebuffer::Framebuffer(VkRenderPass renderPass, std::initializer_list<RenderTargetHandle<>> targets)
		: m_targets(targets)
	{
		uint32_t imageCount = 0;
		uint32_t swapchainCount = 0;
		for (auto& target : m_targets) {
			imageCount = std::max(imageCount, target->getImageCount());
			if (target->getImageCount() > 1)
				swapchainCount++;
		}
		ZP_ASSERT((swapchainCount <= 1), "Framebuffer supports only one swapchain at a time");

		m_framebuffers.resize(imageCount);
		uint32_t index = 0;
		for (auto& framebuffer : m_framebuffers) {
			for(auto& target : m_targets)
				framebuffer.addAttachment(target->getImageView(index));
			framebuffer.setRenderPass(renderPass);
			framebuffer.init();
			index++;
		}
	}

	Framebuffer::~Framebuffer() {
		for (auto& framebuffer : m_framebuffers) {
			framebuffer.destroy();
		}
	}

	void Framebuffer::update() {
		uint32_t index = 0;
		for (auto& framebuffer : m_framebuffers) {
			for (auto& target : m_targets) {
				framebuffer.delAttachment(0);
				framebuffer.addAttachment(target->getImageView(index));
			}
			framebuffer.update();
			index++;
		}
	}

	Framebuffer::operator VkFramebuffer() {
		if (m_framebuffers.size() <= 0) {
			ZP_WARN(false, "Empty framebuffer");
			return VK_NULL_HANDLE;
		}
		for (auto& target : m_targets) {
			int index = target->getImageIndex();
			if (index >= 0 && m_framebuffers.size() > index) {
				return m_framebuffers[index];
			}
		}
		return m_framebuffers[0];
	}
}