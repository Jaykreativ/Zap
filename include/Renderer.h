#pragma once

#include "glm.hpp"
#include "Zap.h"
#include "Window.h"
#include "Vertex.h"
#include "VisibleActor.h"
#include "Camera.h"

namespace Zap {
	class Renderer
	{
	public:
		Renderer(Window& window);
		~Renderer();
	
		virtual void init() = 0;

		virtual void render(Camera* cam) = 0;

		void addActor(VisibleActor& actor);

		void setViewport(uint32_t width, uint32_t height, uint32_t x, uint32_t y);

	protected:
		bool m_isInit = false;

		Window& m_window;
		VkViewport m_viewport;
		VkRect2D m_scissor;
		std::vector<VisibleActor*> m_actors;
	};
}

