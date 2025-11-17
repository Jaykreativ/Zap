#pragma once

#include "Zap/Zap.h"

namespace Zap {
	class ResizeEvent;

	class RenderTarget {
		friend class Renderer;
	public:
		RenderTarget() = default;
		virtual ~RenderTarget() = default;

		void resize(glm::vec2 size);

		bool isValid();

		virtual VkExtent3D getExtent() = 0;

		virtual uint32_t getImageCount();

		// returns index of currently active image in swapchain
		// returns -1 if the target has no swapchain
		virtual int getImageIndex();

		virtual VkImageView getImageView(uint32_t index) = 0;
	protected:
		Renderer* m_pRenderer = nullptr;

		virtual void resizeInternal(glm::vec2 size) = 0;
	private:
		bool m_isValid = false;
	};

	// handle to a RenderTarget stored in a Renderer
	// invalid if the Renderer was destroyed
	template<class T = RenderTarget>
	class RenderTargetHandle {
		friend class Renderer;
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
			return m_renderer->getRenderTarget(m_handle);
		}
		const T* get() const {
			return m_renderer->getRenderTarget(m_handle);
		}

		T* operator->() {
			return get();
		}

		operator bool() const {
			return m_renderer != nullptr && get() != nullptr;
		}

	private:
		UUID m_handle = 0;
		Renderer* m_renderer = nullptr;

		RenderTargetHandle(UUID handle, Renderer* renderer)
			: m_handle(handle), m_renderer(renderer)
		{}
	};

	// wrapper for a standart Zap image used by rendering
	class RenderTargetImage : public RenderTarget {
	public:
		RenderTargetImage();
		~RenderTargetImage();

		virtual VkExtent3D getExtent() override;

		virtual VkImageView getImageView(uint32_t index) override;

	protected:
		void resizeInternal(glm::vec2 size) override;
	private:
		Image m_image;
	};

	// references a Zap window and allows rendering to it
	class RenderTargetWindow : public RenderTarget {
	public:
		RenderTargetWindow(Window& window);
		~RenderTargetWindow();

		virtual VkExtent3D getExtent() override;

		virtual uint32_t getImageCount() override;

		virtual int getImageIndex() override;

		virtual VkImageView getImageView(uint32_t index) override;

	protected:
		void resizeInternal(glm::vec2 size) override;
	private:
		Window& m_window;
	
		static void resizeCallback(ResizeEvent& eventParams, void* customParams);
	};

}