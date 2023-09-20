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
	bool forward = false;
	bool backward = false;
	bool left = false;
	bool right = false;
	bool down = false;
	bool up = false;
	bool lookUp = false;
	bool lookDown = false;
	bool lookLeft = false;
	bool lookRight = false;
	void move(float dTime) {
		if (forward) {
			//app::cam.translate(glm::vec3{0, 0, 1} * dTime * 2.0f);
			auto res = app::cam.getTransform();
			glm::vec3 vec = res[2];
			res[3] = glm::vec4(glm::vec3(res[3]) + glm::normalize(glm::vec3{ vec.x, 0, vec.z })*dTime * 2.0f, 1);
			app::cam.setTransform(res);
		}
		if (backward) {
			auto res = app::cam.getTransform();
			glm::vec3 vec = -res[2];
			res[3] = glm::vec4(glm::vec3(res[3]) + glm::normalize(glm::vec3{ vec.x, 0, vec.z })*dTime * 2.0f, 1);
			app::cam.setTransform(res);
		}
		if (right) {
			auto res = app::cam.getTransform();
			glm::vec3 vec = res[0];
			res[3] = glm::vec4(glm::vec3(res[3]) + glm::normalize(glm::vec3{vec.x, 0, vec.z })*dTime * 2.0f, 1);
			app::cam.setTransform(res);
		}
		if (left) {
			auto res = app::cam.getTransform();
			glm::vec3 vec = -res[0];
			res[3] = glm::vec4(glm::vec3(res[3]) + glm::normalize(glm::vec3{ vec.x, 0, vec.z })*dTime * 2.0f, 1);
			app::cam.setTransform(res);
		}
		if (down) {
			auto res = app::cam.getTransform();
			res[3] = glm::vec4(glm::vec3(res[3]) + glm::vec3{ 0, 2, 0 }*dTime, 1);
			app::cam.setTransform(res);
		}
		if (up) {
			auto res = app::cam.getTransform();
			res[3] = glm::vec4(glm::vec3(res[3]) + glm::vec3{ 0, -2, 0 }*dTime, 1);
			app::cam.setTransform(res);
		}
		if (lookLeft) {
			glm::mat4 res = app::cam.getTransform();
			glm::mat4 rot = glm::rotate(glm::mat4(1), glm::radians<float>(-90 * dTime), glm::vec3{0, 1, 0});

			res[0] = rot * res[0];
			res[1] = rot * res[1];
			res[2] = rot * res[2];

			app::cam.setTransform(res);
		}
		if (lookRight) {
			glm::mat4 res = app::cam.getTransform();
			glm::mat4 rot = glm::rotate(glm::mat4(1), glm::radians<float>(90 * dTime), glm::vec3{ 0, 1, 0 });

			res[0] = rot * res[0];
			res[1] = rot * res[1];
			res[2] = rot * res[2];

			app::cam.setTransform(res);
		}
		if (lookDown) {
			app::cam.rotateX(-90 * dTime);
		}
		if (lookUp) {
			app::cam.rotateX(90 * dTime);
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
	int lookUp = GLFW_KEY_UP;
	int lookDown = GLFW_KEY_DOWN;
	int lookLeft = GLFW_KEY_LEFT;
	int lookRight = GLFW_KEY_RIGHT;
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
			else if (key == lookUp) {
				movement::lookUp = true;
			}
			else if (key == lookDown) {
				movement::lookDown = true;
			}
			else if (key == lookLeft) {
				movement::lookLeft = true;
			}
			else if (key == lookRight) {
				movement::lookRight = true;
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
			else if (key == lookUp) {
				movement::lookUp = false;
			}
			else if (key == lookDown) {
				movement::lookDown = false;
			}
			else if (key == lookLeft) {
				movement::lookLeft = false;
			}
			else if (key == lookRight) {
				movement::lookRight = false;
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
		Vertex({0.5, 0.5, 0.5}),//0
		Vertex({0.5, -0.5, 0.5}),//1
		Vertex({-0.5, -0.5, 0.5}),//2
		Vertex({-0.5, 0.5, 0.5}),//3
		Vertex({0.5, 0.5, -0.5}),//4
		Vertex({0.5, -0.5, -0.5}),//5
		Vertex({-0.5, -0.5, -0.5}),//6
		Vertex({-0.5, 0.5, -0.5})//7
	};
	std::vector<uint32_t> indices = {
		0, 1, 2,//back
		0, 2, 3,
		4, 5, 6,//front
		4, 6, 7,
		0, 1, 5,//right
		0, 5, 4,
		3, 2, 6,//left
		3, 6, 7,
		1, 2, 6,//top
		1, 6, 5,
		0, 3, 7,//bottom
		0, 7, 4
	};

	Zap::Model model = Zap::Model();
	model.load(vertices, indices);

	//Actors
	Zap::VisibleActor centre;
	centre.setModel(model);
	centre.m_color = { 0.5, 0.5, 0.5 };

	Zap::VisibleActor xDir;
	xDir.setModel(model);
	xDir.setPos(1, 0, 0);
	xDir.m_color = { 1, 0, 0 };

	Zap::VisibleActor yDir;
	yDir.setModel(model);
	yDir.setPos(0, 1, 0);
	yDir.m_color = { 0, 1, 0 };

	Zap::VisibleActor zDir;
	zDir.setModel(model);
	zDir.setPos(0, 0, 1);
	zDir.m_color = { 0, 0, 1 };

	app::renderer.addActor(centre);
	app::renderer.addActor(xDir);
	app::renderer.addActor(yDir);
	app::renderer.addActor(zDir);

	app::renderer.setViewport(1000, 600, 0, 0);
	app::renderer.init();

	app::cam.setPos(0, 0, 2);

	//mainloop
	uint64_t currentFrame = 0;
	float dTime = 0;
	while (!app::window.shouldClose()) {
		auto timeStartFrame = std::chrono::high_resolution_clock::now();
		movement::move(dTime);

		app::renderer.render(&app::cam);

		app::window.swapBuffers();
		Zap::Window::pollEvents();
		currentFrame++;
		auto timeEndFrame = std::chrono::high_resolution_clock::now();
		dTime = std::chrono::duration_cast<std::chrono::duration<float>>(timeEndFrame - timeStartFrame).count();
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