#include "Renderer.h"

namespace Zap {
	Renderer::Renderer(Window& window)
		: m_window(window)
	{}

	Renderer::~Renderer() {}

	void Renderer::addActor(VisibleActor& actor) {
		m_actors.push_back(&actor);
	}

	void Renderer::setViewport(uint32_t width, uint32_t height, uint32_t x, uint32_t y){
		m_viewport.x = x;
		m_viewport.y = y;
		m_viewport.width = width;
		m_viewport.height = height;
		m_viewport.minDepth = 0;
		m_viewport.maxDepth = 1;

		m_scissor.offset.x = x;
		m_scissor.offset.y = y;
		m_scissor.extent.width = width;
		m_scissor.extent.height = height;
		}
}