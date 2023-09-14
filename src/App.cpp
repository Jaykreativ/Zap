#include "Zap.h"
#include "Window.h"
#include "Renderer.h"
#include "PxPhysicsAPI.h"
#include "glm.hpp"
#include "gtc/matrix_transform.hpp"

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

	Zap::Model model = Zap::Model();
	model.load(vertices, indices);

	Zap::VisibleActor actor;
	actor.setModel(model);
	actor.setTransform(glm::rotate(*actor.getTransform(), glm::radians<float>(45), glm::vec3{ 0, 0, 1 }));

	Zap::VisibleActor actor2;
	actor2.setModel(model);
	actor2.setTransform(glm::rotate(*actor2.getTransform(), glm::radians<float>(0), glm::vec3{ 0, 0, 1 }));

	app::renderer.addActor(actor);
	app::renderer.addActor(actor2);

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