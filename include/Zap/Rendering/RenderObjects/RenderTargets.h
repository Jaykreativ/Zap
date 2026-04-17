#pragma once

#include "Zap/Zap.h"
#include "Zap/Rendering/Image.h"
#include "Zap/Rendering/RenderObject.h"

namespace Zap {
	class RenderTarget : public RenderObject {
		friend class Renderer;
	public:
		RenderTarget(Renderer* pRenderer);
		virtual ~RenderTarget() = default;

		void resize(glm::vec2 size);

		virtual void recLayoutTransition(const vk::CommandBuffer& cmd, VkImageLayout oldLayout, VkImageLayout newLayout, VkAccessFlags srcAccessMask, VkAccessFlags dstAccessMask) = 0;

		bool isValid();

		void setInitialLayout(VkImageLayout initialLayout);
		void setFinalLayout(VkImageLayout finalLayout);

		VkImageLayout getInitialLayout();
		VkImageLayout getFinalLayout();

		virtual VkExtent3D getExtent() = 0;

		virtual uint32_t getImageCount();

		// returns index of currently active image in swapchain
		// returns -1 if the target has no swapchain
		virtual int getImageIndex();

		virtual VkImageView getImageView(uint32_t index) = 0;

		virtual void resizeInternal(VkExtent2D extent) = 0;
	private:
		bool m_isValid = false;

		VkImageLayout m_initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		VkImageLayout m_finalLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	};

	// handle to a RenderTarget stored in a Renderer
	// invalid if the Renderer was destroyed
	template<class T = RenderTarget>
	class RenderTargetHandle {
		friend class Renderer;
		friend class LayoutTransitionHelper;
		template<class U>
		friend class RenderTargetHandle;
	public:
		RenderTargetHandle() = default;
		RenderTargetHandle(const RenderTargetHandle<T>& other)
			: m_handle(other.m_handle), m_renderer(other.m_renderer)
		{}
		~RenderTargetHandle() = default;

		operator RenderTargetHandle<RenderTarget>() {
			return RenderTargetHandle<RenderTarget>(m_handle, m_renderer);
		}

		T* get() {
			return reinterpret_cast<T*>(m_renderer->getRenderTarget(m_handle));
		}
		const T* get() const {
			return reinterpret_cast<T*>(m_renderer->getRenderTarget(m_handle));
		}

		T* operator->() {
			return get();
		}

		operator bool() const {
			return m_renderer != nullptr && get() != nullptr;
		}

		void reset() {
			m_handle = 0;
			m_renderer = nullptr;
		}

	private:
		UUID m_handle = 0;
		Renderer* m_renderer = nullptr;

		RenderTargetHandle(UUID handle, Renderer* renderer)
			: m_handle(handle), m_renderer(renderer)
		{}
	};

	// wrapper for a standart Zap image written to during rendering
	// completely owns the underlying image resource which can be accessed through the renderer using extractRenderTargetImage() which destroyes the RenderTarget
	class RenderTargetImage : public RenderTarget {
	public:
		RenderTargetImage(Renderer* pRenderer, VkFormat format, VkImageUsageFlags usage, VkMemoryPropertyFlags memoryProperties);
		~RenderTargetImage();

		virtual void recLayoutTransition(const vk::CommandBuffer& cmd, VkImageLayout oldLayout, VkImageLayout newLayout, VkAccessFlags srcAccessMask, VkAccessFlags dstAccessMask) override;

		virtual VkExtent3D getExtent() override;

		virtual VkImageView getImageView(uint32_t index = 0) override;

	protected:
		virtual void resizeInternal(VkExtent2D size) override;
	private:
		Image2D m_image;

		friend class Renderer;
	};

	class RenderTargetGuiImage : public RenderTargetImage {
	public:
		RenderTargetGuiImage(Renderer* pRenderer, VkFormat format, VkImageUsageFlags usage, VkMemoryPropertyFlags memoryProperties);
		~RenderTargetGuiImage();

		operator VkDescriptorSet() { return m_imageDescriptorSet; }

		const vk::Sampler& getSampler() { return m_sampler; }

		VkDescriptorSet getDescriptorSet() { return m_imageDescriptorSet; }

	protected:
		virtual void resizeInternal(VkExtent2D size) override;

	private:
		vk::Sampler m_sampler;
		VkDescriptorSet m_imageDescriptorSet;
	};

	// references a Zap window and allows rendering to it
	class RenderTargetWindow : public RenderTarget,
		public EventListener<WindowEvent::Resize>
	{
	public:
		RenderTargetWindow(Renderer* pRenderer, Window& window);
		~RenderTargetWindow();

		virtual void recLayoutTransition(const vk::CommandBuffer& cmd, VkImageLayout oldLayout, VkImageLayout newLayout, VkAccessFlags srcAccessMask, VkAccessFlags dstAccessMask) override;

		virtual VkExtent3D getExtent() override;

		virtual uint32_t getImageCount() override;

		virtual int getImageIndex() override;

		virtual VkImageView getImageView(uint32_t index) override;

	protected:
		void resizeInternal(VkExtent2D extent) override;
	private:
		Window& m_window;
	
		void callback(const WindowEvent::Resize& event) override;
	};

}