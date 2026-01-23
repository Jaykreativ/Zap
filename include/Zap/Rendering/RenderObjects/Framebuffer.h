#pragma once

#include "Zap/Rendering/RenderObject.h"
#include "Zap/Rendering/RenderEvents.h"
#include "Zap/Rendering/RenderObjects/RenderTargets.h"

#include <vector>

namespace Zap {
	class Framebuffer : virtual public RenderObject, public RenderEventListener<RenderEvent::Resize> {
	public:
		Framebuffer(Renderer* pRenderer, VkRenderPass renderPass, std::initializer_list<RenderTargetHandle<>> targets);
		~Framebuffer();

		// recreates internal framebuffers using the same targets
		void update();

		virtual void callback(const RenderEvent::Resize& event) override;

		operator VkFramebuffer();

	private:
		std::vector<vk::Framebuffer> m_framebuffers = {};
		std::vector<RenderTargetHandle<>> m_targets;
	};

	class FramebufferHandle {
		friend class Renderer;
	public:
		FramebufferHandle(){}
		FramebufferHandle(const FramebufferHandle& other)
			: m_handle(other.m_handle), m_renderer(other.m_renderer)
		{}
		~FramebufferHandle(){}

		Framebuffer* get();
		const Framebuffer* get() const;

		Framebuffer* operator->();

		operator bool() const;

	private:
		UUID m_handle = 0;
		Renderer* m_renderer = nullptr;

		FramebufferHandle(UUID handle, Renderer* renderer)
			: m_handle(handle), m_renderer(renderer)
		{}
	};
}