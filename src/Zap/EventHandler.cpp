#include "Zap/EventHandler.h"

std::unordered_map<int, bool> keyStates;
std::unordered_map<int, bool> mouseButtonStates;
std::vector<void (*)(GLFWwindow*, double, double, void*)> cursorPosCallbacks;
std::vector<void*> cursorPosCallbackData;

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	if (action == GLFW_PRESS) {
		keyStates[key] = true;
	}
	else if (action == GLFW_RELEASE) {
		keyStates[key] = false;
	}
}

void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
	if (action == GLFW_PRESS) {
		mouseButtonStates[button] = true;
	}
	else if (action == GLFW_RELEASE) {
		mouseButtonStates[button] = false;
	}
}

void cursorPositionCallback(GLFWwindow* window, double xpos, double ypos) {
	uint32_t i = 0;
	for (void (*fun)(GLFWwindow*, double, double, void*) : cursorPosCallbacks)
		fun(window, xpos, ypos, cursorPosCallbackData[i]); i++;
}

namespace Zap {
	EventHandler::EventHandler(Zap::Window* window)
		: m_window(window)
	{
		m_window->setKeyCallback(keyCallback);
		m_window->setMousebButtonCallback(mouseButtonCallback);
		m_window->setCursorPosCallback(cursorPositionCallback);
	}

	EventHandler::~EventHandler() {}

	bool EventHandler::isKeyPressed(int key) {
		return keyStates[key];
	}

	bool EventHandler::isMouseButtonPressed(int mouseButton) {
		return mouseButtonStates[mouseButton];
	}

	void EventHandler::addCursorPositionCallback(void (*cursorPosCallback)(GLFWwindow*, double, double, void*), void* data) {
		cursorPosCallbacks.push_back(cursorPosCallback);
		cursorPosCallbackData.push_back(data);
	}

}
