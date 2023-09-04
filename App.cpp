#include "Zap.h"
#include "Window.h"
#include "Renderer.h"

int main() {
	Zap::init("Zap Application");

	Zap::Window window = Zap::Window(1000, 600, "Zap Application");
	window.init();
	window.show();

	Zap::Renderer renderer = Zap::Renderer(window);
	renderer.setViewport(1000, 600, 0, 0);
	renderer.init();

	while (!window.shouldClose()) {
		renderer.render();

		Zap::Window::pollEvents();
	}

	renderer.~Renderer();
	window.~Window();

	Zap::terminate();

	system("pause");
	return 0;
}