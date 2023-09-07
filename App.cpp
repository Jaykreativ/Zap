#include "Zap.h"
#include "Window.h"
#include "Renderer.h"

namespace app {
	Zap::Window window = Zap::Window(1000, 600, "Zap Application");

	Zap::Renderer renderer = Zap::Renderer(window);
	Zap::Renderer renderer2 = Zap::Renderer(window);
}

int main() {
	Zap::init("Zap Application");

	
	app::window.init();
	app::window.show();


	app::renderer.setViewport(1000, 600, 0, 0);
	app::renderer.init();

	app::renderer2.setViewport(500, 300, 500, 0);
	app::renderer2.init();

	while (!app::window.shouldClose()) {
		app::renderer.render();
		app::renderer2.render();

		app::window.swapBuffers();
		Zap::Window::pollEvents();
	}

	app::renderer.~Renderer();
	app::renderer2.~Renderer();
	app::window.~Window();

	Zap::terminate();

#ifdef _DEBUG
	system("pause");
#endif
	return 0;
}