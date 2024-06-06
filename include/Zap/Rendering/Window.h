#pragma once

#include "Zap/Zap.h"
#include "Zap/EventHandler.h"
#include "Zap/Event.h"

namespace Zap
{
	class Renderer;
	class Window;

	class ResizeEvent : public Event {
	public:
		ResizeEvent(Window* pWindow, int width, int height)
			: pWindow(pWindow), width(width), height(height)
		{};
		~ResizeEvent() = default;

		Window* pWindow = nullptr;
		int width = 0;
		int height = 0;
	};

	class KeyEvent : public Event {
	public:
		KeyEvent(Window* pWindow, int key, int scancode, int action, int mods)
			: pWindow(pWindow), key(key), scancode(scancode), action(action), mods(mods)
		{}
		~KeyEvent() = default;

		Window* pWindow = nullptr;
		int key = 0;
		int scancode = 0;
		int action = 0;
		int mods = 0;
	};

	class CursorPosEvent : public Event {
	public:
		CursorPosEvent(Window* pWindow, double xPos = 0, double yPos = 0)
			: pWindow(pWindow), xPos(xPos), yPos(yPos)
		{}
		~CursorPosEvent() = default;

		Window* pWindow = nullptr;
		double xPos = 0;
		double yPos = 0;
	};

	class MouseButtonEvent : public Event {
	public:
		MouseButtonEvent(Window* pWindow, int button, int action, int mods)
			: pWindow(pWindow), button(button), action(action), mods(mods)
		{}
		~MouseButtonEvent() = default;

		//TODO add Zap intern System for button indexing
		Window* pWindow = nullptr;
		int button = 0;
		int action = 0;
		int mods = 0;
	};

	class ScrollEvent : public Event {
	public:
		ScrollEvent(Window* pWindow, double xoffset, double yoffset)
			: pWindow(pWindow), xoffset(xoffset), yoffset(yoffset)
		{}
		~ScrollEvent() = default;

		Window* pWindow = nullptr;
		double xoffset = 0;
		double yoffset = 0;
	};

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

		EventHandler<ResizeEvent>* getResizeEventHandler();
		EventHandler<KeyEvent>* getKeyEventHandler();
		EventHandler<CursorPosEvent>* getCursorPosEventHandler();
		EventHandler<MouseButtonEvent>* getMouseButtonEventHandler();
		EventHandler<ScrollEvent>* getScrollEventHandler();

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

		EventHandler<ResizeEvent> m_resizeEventHandler;
		EventHandler<KeyEvent> m_keyEventHandler;
		EventHandler<CursorPosEvent> m_cursorPosEventHandler;
		EventHandler<MouseButtonEvent> m_mouseButtonEventHandler;
		EventHandler<ScrollEvent> m_scrollEventHandler;

		Renderer* m_renderer = nullptr;

		void resize(GLFWwindow* window, int width, int height);

		// maps glfw window ptr to the corresponding zap window ptr
		static std::unordered_map<GLFWwindow*, Window*> glfwWindowMap;
		static void resizeGLFWCallback(GLFWwindow* window, int width, int height);
		static void keyGLFWCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
		static void cursorPosGLFWCallback(GLFWwindow* window, double xPos, double yPos);
		static void mouseButtonGLFWCallback(GLFWwindow* window, int button, int action, int mods);
		static void scrollGLFWCallback(GLFWwindow* window, double xoffset, double yoffset);

		static void resizeEventCallback(Zap::ResizeEvent& resizeEvent, void* data);

		friend class Renderer;
		friend class PBRenderer;
		friend class Gui;
	};
}