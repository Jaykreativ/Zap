#include "Zap/Zap.h"
#include "Zap/Window.h"
#include "Zap/Renderer.h"
#include "Zap/PBRenderer.h"
#include "Zap/Gui.h"
#include "Zap/Scene/Scene.h"
#include "Zap/Scene/Mesh.h"
#include "Zap/Scene/Shape.h"
#include "Zap/Scene/Actor.h"
#include "Zap/Scene/Component.h"
#include "Zap/Scene/Transform.h"
#include "Zap/Scene/MeshComponent.h"
#include "imgui.h"
#include "PxPhysicsAPI.h"
#include "glm.hpp"
#include "gtc/matrix_transform.hpp"
#include "gtc/type_ptr.hpp"

namespace app {
	Zap::Base* engineBase = Zap::Base::createBase("Zap Application");

	Zap::Window window = Zap::Window(1000, 600, "Zap Window");

	Zap::Gui gui = Zap::Gui(window);

	Zap::PBRenderer renderer = Zap::PBRenderer(window);
	Zap::PBRenderer renderer2 = Zap::PBRenderer(window);

	Zap::Actor cam = Zap::Actor();
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
			res[3] = glm::vec4(glm::vec3(res[3]) + glm::vec3{ 0, -2, 0 }*dTime, 1);
			app::cam.setTransform(res);
		}
		if (up) {
			auto res = app::cam.getTransform();
			res[3] = glm::vec4(glm::vec3(res[3]) + glm::vec3{ 0, 2, 0 }*dTime, 1);
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
			app::cam.getTransformComponent()->rotateX(90 * dTime);
		}
		if (lookUp) {
			app::cam.getTransformComponent()->rotateX(-90 * dTime);
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
			/*else if (key == GLFW_KEY_ENTER) {
				glm::vec3 dir = app::cam.getTransform()[2];
				px::cubePxActor->addForce(PxVec3(dir.x*500, dir.y*500, dir.z*500));
				//px::cubePxActor->setGlobalPose(PxTransform(PxVec3(0, 5, 0)));
			}*/
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

void resize(GLFWwindow* window, int width, int height) {
	app::renderer.setViewport(width, height, 0, 0);
	app::gui.setViewport(width, height, 0, 0);
}

int main() {
	app::engineBase->init();

	app::window.init();
	app::window.show();
	app::window.setKeyCallback(keybinds::keyCallback);
	app::window.setResizeCallback(resize);

	app::gui.setViewport(app::window.getWidth(), app::window.getHeight(), 0, 0);
	app::gui.init();

	Zap::Mesh model = Zap::Mesh();
	model.load("Models/OBJ/Cube.obj");

	//Zap::Model sponzaModel = Zap::Model();
	//sponzaModel.load("Models/OBJ/Sponza.obj");

	Zap::Mesh giftModel = Zap::Mesh();
	giftModel.load("Models/OBJ/Gift.obj");

	//Actors
	Zap::Actor centre;
	centre.addTransform(glm::mat4(1));
	centre.getTransformComponent()->setPos(0, 0, 0);
	centre.getTransformComponent()->setScale(0.25, 0.25, 0.25);
	centre.addMesh(&model);

	Zap::Actor xDir;
	xDir.addTransform(glm::mat4(1));
	xDir.getTransformComponent()->setPos(0.75, 0, 0);
	xDir.getTransformComponent()->setScale(0.5, 0.1, 0.1);
	xDir.addMesh(&model);
	xDir.getMeshComponent(0)->m_material.m_AlbedoColor = { 1, 0, 0 };

	Zap::Actor yDir;
	yDir.addTransform(glm::mat4(1));
	yDir.getTransformComponent()->setPos(0, 0.75, 0);
	yDir.getTransformComponent()->setScale(0.1, 0.5, 0.1);
	yDir.addMesh(&model);
	yDir.getMeshComponent(0)->m_material.m_AlbedoColor = { 0, 1, 0 };

	Zap::Actor zDir;
	zDir.addTransform(glm::mat4(1));
	zDir.getTransformComponent()->setPos(0, 0, 0.75);
	zDir.getTransformComponent()->setScale(0.1, 0.1, 0.5);
	zDir.addMesh(&model);
	zDir.getMeshComponent(0)->m_material.m_AlbedoColor = { 0, 0, 1 };

	auto pxMaterial = Zap::PhysicsMaterial(0.5, 0.5, 0.6);

	Zap::Actor physicstest;
	physicstest.addTransform(glm::mat4(1));
	physicstest.getTransformComponent()->setPos({ 0, 5, 0 });
	physicstest.getTransformComponent()->setScale({ 0.5, 0.5, 0.5 });
	{
		auto geometry = Zap::BoxGeometry({ 1, 1, 1 });
		auto shape = Zap::Shape(geometry, pxMaterial, true);
		physicstest.addPhysics(Zap::PHYSICS_TYPE_RIGID_DYNAMIC, shape);
	}

	physicstest.addCamera({0, 0, 0});
	physicstest.addMesh(&giftModel);
	physicstest.addLight({ 0.25, 1, 3 });

	Zap::Actor rotatingGift;
	rotatingGift.addTransform(glm::mat4(1));
	rotatingGift.getTransformComponent()->setPos(3, 2, 2);
	rotatingGift.addMesh(&giftModel);
	rotatingGift.getMeshComponent(0)->m_material.m_AlbedoColor = {0.5, 1, 0.5};

	Zap::Actor ground;
	ground.addTransform(glm::mat4(1));
	ground.getTransformComponent()->setPos(0, -2, 0);
	ground.getTransformComponent()->setScale(500, 1, 500);
	{
		auto geometry = Zap::PlaneGeometry();
		glm::mat4 localTransform = glm::mat4(1);
		localTransform = glm::translate(localTransform, glm::vec3(0, 1, 0));
		localTransform =  glm::rotate(localTransform, glm::radians<float>(90), glm::vec3(0, 0, 1));
		auto shape = Zap::Shape(geometry, pxMaterial, true, localTransform);
		ground.addPhysics(Zap::PHYSICS_TYPE_RIGID_STATIC, shape);
	}
	ground.addMesh(&model);

	Zap::Actor skybox;
	skybox.addTransform(glm::mat4(1));
	skybox.getTransformComponent()->setPos(0, 0, 0);
	skybox.getTransformComponent()->setScale(500, 500, 500);
	skybox.addMesh(&model);

	Zap::Actor light;
	light.addTransform(glm::mat4(1));
	light.getTransformComponent()->setPos({ -3, 2, 0 });
	light.addLight({ 2.5, 2.5, 2.5 });

	Zap::Actor light2;
	light2.addTransform(glm::mat4(1));
	light2.getTransformComponent()->setPos({ 3, 2, 0 });
	light2.addLight({ 3, 1.5, 0.6 });

	app::cam.addTransform(glm::mat4(1));
	app::cam.getTransformComponent()->setPos(-1, 1, -5);
	app::cam.addCamera(glm::vec3(0, 0, 0));

	app::renderer.setViewport(1000, 600, 0, 0);
	app::renderer.init();

	app::renderer2.setViewport(500, 300, 0, 0);
	app::renderer2.init();

	//mainloop
	float dTime = 0;
	while (!app::window.shouldClose()) {
		auto timeStartFrame = std::chrono::high_resolution_clock::now();
		movement::move(dTime);

		rotatingGift.getTransformComponent()->rotateY(45 * dTime);

		ImGui::ShowDemoWindow();

		if (dTime > 0) {
			Zap::Scene::simulate(dTime);
		}

		app::window.clear();

		app::renderer.render(app::cam.getComponentIDs(Zap::COMPONENT_TYPE_CAMERA)[0]);
		app::window.clearDepthStencil();
		app::renderer2.render(physicstest.getComponentIDs(Zap::COMPONENT_TYPE_CAMERA)[0]);
		app::window.clearDepthStencil();
		app::gui.render(0);

		app::window.swapBuffers();
		Zap::Window::pollEvents();
		auto timeEndFrame = std::chrono::high_resolution_clock::now();
		dTime = std::chrono::duration_cast<std::chrono::duration<float>>(timeEndFrame - timeStartFrame).count();
	}

	//terminate
	app::renderer.~PBRenderer();
	app::renderer2.~PBRenderer();
	app::gui.~Gui();
	app::window.~Window();

	app::engineBase->terminate();

#ifdef _DEBUG
	system("pause");
#endif
	return 0;
}