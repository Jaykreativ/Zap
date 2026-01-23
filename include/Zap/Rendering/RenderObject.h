#pragma once

namespace Zap {
	class Renderer;
	class RenderEventHandler;
	class RenderObject {
	public:
		RenderObject(Renderer* pRenderer);

		RenderEventHandler& getEventHandler();

	protected:
		Renderer* m_pRenderer;
	};
}