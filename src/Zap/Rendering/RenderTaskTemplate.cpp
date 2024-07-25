#include "Zap/Rendering/RenderTaskTemplate.h"

#include "Zap/Rendering/Renderer.h"

namespace Zap {
	void RenderTaskTemplate::initTargetDependencies() { m_pRenderer->initRenderTaskTargetDependencies(this); }

	void RenderTaskTemplate::resizeTargetDependencies() { m_pRenderer->resizeRenderTaskTargetDependencies(this); }
}