#include "Zap.h"
#include "Window.h"
#include "Renderer.h"
#include "PxPhysicsAPI.h"
#include "glm.hpp"
#include "gtc/matrix_transform.hpp"

namespace app {
	Zap::Window window = Zap::Window(1000, 600, "Zap Application");

	Zap::Renderer renderer = Zap::Renderer(window);

	Zap::Camera cam = Zap::Camera();
}

namespace movement {
	float speed = 0.01;

	bool forward = false;
	bool backward = false;
	bool left = false;
	bool right = false;
	bool down = false;
	bool up = false;
	void move() {
		if (forward) {
			app::cam.translate(-glm::vec3(app::cam.getTransform()[2]) * speed);
		}
		if (backward) {
			app::cam.translate(glm::vec3(app::cam.getTransform()[2]) * speed);
		}
		if (left) {
			app::cam.translate(-glm::vec3(app::cam.getTransform()[0]) * speed);
		}
		if (right) {
			app::cam.translate(glm::vec3(app::cam.getTransform()[0]) * speed);
		}
		if (down) {
			app::cam.translate(glm::vec3(app::cam.getTransform()[1]) * speed);
		}
		if (up) {
			app::cam.translate(-glm::vec3(app::cam.getTransform()[1]) * speed);
		}
	}
}

namespace keybinds {
	int forward = GLFW_KEY_W;
	int backward = GLFW_KEY_S;
	int left = GLFW_KEY_A;
	int right = GLFW_KEY_D;
	int down = GLFW_KEY_C;
	int up = GLFW_KEY_SPACE;
	void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
		if (action == GLFW_PRESS) {
			if (key == forward) {
				movement::forward = true;
			}
			else if (key == backward) {
				movement::backward = true;
			}
			else if (key == left) {
				movement::left = true;
			}
			else if (key == right) {
				movement::right = true;
			}
			else if (key == down) {
				movement::down = true;
			}
			else if (key == up) {
				movement::up = true;
			}
		}
		else if(action == GLFW_RELEASE) {
			if (key == forward) {
				movement::forward = false;
			}
			else if (key == backward) {
				movement::backward = false;
			}
			else if (key == left) {
				movement::left = false;
			}
			else if (key == right) {
				movement::right = false;
			}
			else if (key == down) {
				movement::down = false;
			}
			else if (key == up) {
				movement::up = false;
			}
		}
	}
}

int main() {
	physx::PxDefaultAllocator gDefaultAllocator;
	physx::PxDefaultErrorCallback gDefaultErrorCallback;

	physx::PxFoundation* foundation = PxCreateFoundation(PX_PHYSICS_VERSION, gDefaultAllocator, gDefaultErrorCallback);

	Zap::init("Zap Application");
	
	app::window.init();
	app::window.show();
	app::window.setKeyCallback(keybinds::keyCallback);

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

	//Actors
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

	app::cam.setPos(0, 0, 2);

	//mainloop
	uint64_t currentFrame = 0;
	while (!app::window.shouldClose()) {
		actor.setTransform(glm::rotate(actor.getTransform(), glm::radians<float>(0.25), glm::vec3{ 0, 0, 1 }));
		//app::cam.setPos(glm::rotate(glm::mat4(1), glm::radians<float>(0.05), glm::vec3(0, 1, 0)) * glm::vec4(app::cam.getPos(), 1));
		movement::move();

		app::renderer.render(&app::cam);

		app::window.swapBuffers();
		Zap::Window::pollEvents();
		currentFrame++;
	}

	//terminate
	app::renderer.~Renderer();
	app::window.~Window();

	Zap::terminate();

#ifdef _DEBUG
	system("pause");
#endif
	return 0;
}