#include "Zap/Rendering/RenderObject.h"

#include "Zap/Rendering/Renderer.h"
#include "Zap/Rendering/RenderEvents.h"

namespace Zap {
	RenderObject::RenderObject(Renderer* pRenderer)
		: m_pRenderer(pRenderer)
	{}

	RenderEventHandler& RenderObject::getEventHandler() {
		return m_pRenderer->m_eventHandler;
	}
}