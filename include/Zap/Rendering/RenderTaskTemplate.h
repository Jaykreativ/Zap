#pragma once

#include "Zap/Zap.h"

namespace Zap {
	class RenderTaskTemplate {
	public:
		void disable() { m_isEnabled = false; }

		void enable() { m_isEnabled = true; }

	protected:
		// Executes the overwritten initTargetDependencies function
		// Can be called in the init function
		void initTargetDependencies();

		// Executes the overwritten resizeTargetDependencies function
		// Can be called in the resize function
		void resizeTargetDependencies();

	private:
		bool m_isEnabled = true;
		Renderer* m_pRenderer = nullptr;

		virtual void init(uint32_t width, uint32_t height, uint32_t imageCount) = 0;

		//Will be imageCount times executed
		//Target extent will remain the same
		virtual void initTargetDependencies(uint32_t width, uint32_t height, uint32_t imageCount, vk::Image* pTarget, uint32_t imageIndex) = 0;

		//Will be called when the target gets resized
		virtual void resize(uint32_t width, uint32_t height, uint32_t imageCount) = 0;

		//Will be called when the target gets resized
		//Will be imageCount times executed
		//Target extent will remain the same
		virtual void resizeTargetDependencies(uint32_t width, uint32_t height, uint32_t imageCount, vk::Image* pTarget, uint32_t imageIndex) = 0;

		virtual void destroy() = 0;

		virtual void beforeRender(vk::Image* pTarget, uint32_t imageIndex) = 0;

		virtual void afterRender(vk::Image* pTarget, uint32_t imageIndex) = 0;

		virtual void recordCommands(const vk::CommandBuffer* cmd, vk::Image* pTarget, uint32_t imageIndex) = 0;

		friend class Renderer;
	};
}