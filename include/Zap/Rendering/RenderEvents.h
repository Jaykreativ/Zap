#pragma once

#include "Zap/Zap.h"
#include "Zap/EventHandler.h"

#include "glm.hpp"

namespace Zap {
	namespace RenderEvent {
		class RenderEvent : public Event {};

		class Resize : public RenderEvent {
		public:
			glm::vec2 size;
		};
	}

	class RenderEventHandler : 
		public EventHandler<RenderEvent::Resize>
	{
	private:
		RenderEventHandler() = default;
		~RenderEventHandler() = default;

		friend class Renderer;
	};
}