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
	actor.setPos(0, 0, 0.5);
	actor.m_color = { 0.2, 0.7, 1 };

	Zap::VisibleActor actor2;
	actor2.setModel(model);
	actor2.m_color = { 1, 0.7, 0.2 };

	app::renderer.addActor(actor);
	app::renderer.addActor(actor2);

	app::renderer.setViewport(1000, 600, 0, 0);
	app::renderer.init();

	Zap::Camera cam = Zap::Camera();
	cam.setPos(0, 0, 2);

	uint64_t currentFrame = 0;
	while (!app::window.shouldClose()) {
		actor.setTransform(glm::rotate(actor.getTransform(), glm::radians<float>(0.25), glm::vec3{ 0, 0, 1 }));
		cam.setPos(glm::rotate(glm::mat4(1), glm::radians<float>(0.05), glm::vec3(0, 1, 0)) * glm::vec4(cam.getPos(), 1));

		app::renderer.render(&cam);

		app::window.swapBuffers();
		Zap::Window::pollEvents();
		currentFrame++;
	}

	app::renderer.~Renderer();
	app::window.~Window();

	Zap::terminate();

#ifdef _DEBUG
	system("pause");
#endif
	return 0;
}