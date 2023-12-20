#pragma once

#include "Zap/Zap.h"

namespace Zap
{
	class Window
	{
	public:
		Window(uint32_t width, uint32_t height, std::string title);
		~Window();

		void init();

		bool shouldClose();

		void show();

		void setKeyCallback(GLFWkeyfun callback);

		void setMousebButtonCallback(GLFWmousebuttonfun mouseButtonCallback);

		void setCursorPosCallback(GLFWcursorposfun cursorPosCallback);

		void setResizeCallback(GLFWwindowsizefun callback);

		/*Getter*/
		uint32_t getWidth();

		uint32_t getHeight();

		static void pollEvents();

	private:
		bool m_isInit = false;

		uint32_t m_width;
		uint32_t m_height;
		std::string m_title;

		GLFWwindow *m_window;
		GLFWwindowsizefun m_sizeCallback = nullptr;

		void resizeVk(GLFWwindow* window, int width, int height);

		static void resizeCallback(GLFWwindow* window, int width, int height);

		friend class Renderer;
		friend class PBRenderer;
		friend class Gui;
	};
}
