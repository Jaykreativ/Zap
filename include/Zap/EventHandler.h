#pragma once

#include "Zap/Rendering/Window.h"

namespace Zap {
	class EventHandler
	{
	public:
		EventHandler(Zap::Window* window);
		~EventHandler();

		bool isKeyPressed(int key);
		bool isMouseButtonPressed(int mouseButton);

		void addCursorPositionCallback(void (* cursorPosCallback)(GLFWwindow*, double, double, void*), void* data);

	private:
		Zap::Window* m_window;
	};
}

