#pragma once

#include "glm.hpp"
#include "Zap/Zap.h"
#include "Window.h"
#include "Zap/Vertex.h"
#include "Zap/Scene/Camera.h"

namespace Zap {
	class Renderer
	{
	public:
		Renderer(Window& window);
		~Renderer();
	
		virtual void init() = 0;

		virtual void recordCommandBuffers() = 0;

		virtual void render(uint32_t cam) = 0;

		void setViewport(uint32_t width, uint32_t height, uint32_t x, uint32_t y);

	protected:
		bool m_isInit = false;

		Window& m_window;
		VkViewport m_viewport;
		VkRect2D m_scissor;
	};
}

