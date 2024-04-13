#pragma once

#include "Zap/Zap.h"

namespace Zap
{
	class Renderer;

	class Window
	{
	public:
		Window(uint32_t width, uint32_t height, std::string title);
		~Window();

		operator GLFWwindow* () { return m_window; }

		void init();

		bool shouldClose();

		bool isIconified();

		void show();

		void setKeyCallback(GLFWkeyfun callback);

		void setMousebButtonCallback(GLFWmousebuttonfun mouseButtonCallback);

		void setScrollCallback(GLFWscrollfun scrollCallback);

		void setCursorPosCallback(GLFWcursorposfun cursorPosCallback);

		void setResizeCallback(GLFWwindowsizefun callback);

		/*Getter*/
		uint32_t getWidth();

		uint32_t getHeight();

		static void pollEvents();

#ifndef ZP_ALL_PUBLIC
	private:
#endif

		bool m_isInit = false;

		uint32_t m_width;
		uint32_t m_height;
		std::string m_title;

		GLFWwindow *m_window;
		GLFWwindowsizefun m_sizeCallback = nullptr;

		Renderer* m_renderer = nullptr;

		void resize(GLFWwindow* window, int width, int height);

		static void resizeCallback(GLFWwindow* window, int width, int height);

		friend class Renderer;
		friend class PBRenderer;
		friend class Gui;
	};
}
