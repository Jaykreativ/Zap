#include "Zap.h"
#include "Window.h"
#include "Renderer.h"
#include "PxPhysicsAPI.h"

namespace app {
	Zap::Window window = Zap::Window(1000, 600, "Zap Application");

	Zap::Renderer renderer = Zap::Renderer(window);
}

int main() {
	physx::PxDefaultAllocator gDefaultAllocator;
	physx::PxDefaultErrorCallback gDefaultErrorCallback;

	physx::PxFoundation* foundation = PxCreateFoundation(PX_PHYSICS_VERSION, gDefaultAllocator, gDefaultErrorCallback);

	Zap::init("Zap Application");
	
	app::window.init();
	app::window.show();

	std::vector<Vertex> vertices = {
		Vertex({-0.5, 0.5, 0}),
		Vertex({0.5, 0.5, 0}),
		Vertex({0, -0.5, 0})
	};
	std::vector<uint32_t> indices = {
		0, 1, 2
	};

	Zap::VisibleActor actor;
	actor.setVertexArray(vertices.data(), vertices.size());
	actor.setIndexArray(indices.data(), indices.size());

	app::renderer.addActor(actor);

	app::renderer.setViewport(1000, 600, 0, 0);
	app::renderer.init();

	while (!app::window.shouldClose()) {
		app::renderer.render();

		app::window.swapBuffers();
		Zap::Window::pollEvents();
	}

	app::renderer.~Renderer();
	app::window.~Window();

	Zap::terminate();

#ifdef _DEBUG
	system("pause");
#endif
	return 0;
}