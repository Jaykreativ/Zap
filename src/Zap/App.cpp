#include "Zap/Zap.h"
#include "Zap/Window.h"
#include "Zap/PBRenderer.h"
#include "Zap/Scene/Mesh.h"
#include "Zap/Scene/Actor.h"
#include "Zap/Scene/Component.h"
#include "Zap/Scene/MeshComponent.h"
#include "PxPhysicsAPI.h"
#include "glm.hpp"
#include "gtc/matrix_transform.hpp"
#include "gtc/type_ptr.hpp"

namespace app {
	Zap::Base engineBase = Zap::Base("Zap Application");

	Zap::Window window = Zap::Window(1000, 600, "Zap Application");

	Zap::PBRenderer renderer = Zap::PBRenderer(window);
	Zap::PBRenderer renderer2 = Zap::PBRenderer(window);

	Zap::Actor cam = Zap::Actor();
}

using namespace physx;
namespace px {
	PxDefaultAllocator gDefaultAllocator;
	PxDefaultErrorCallback gDefaultErrorCallback;
	PxSimulationFilterShader gDefaultFilterShader;

	PxFoundation* foundation;
	PxPvd* pvd;
	PxPhysics* physics;

	PxScene* scene;
	PxRigidDynamic* cubePxActor;

	void init() {
		foundation = PxCreateFoundation(PX_PHYSICS_VERSION, gDefaultAllocator, gDefaultErrorCallback);
		if (!foundation) {
			std::cerr << "ERROR: PxCreateFoundation failed\n";
			throw std::runtime_error("ERROR: PxCreateFoundation failed");
		}

		/*pvd = PxCreatePvd(*foundation);
		PxPvdTransport* transport = PxDefaultPvdSocketTransportCreate("127.0.0.1", 5425, 10);
		pvd->connect(*transport, PxPvdInstrumentationFlag::eALL);*/

		physics = PxCreatePhysics(PX_PHYSICS_VERSION, *foundation, PxTolerancesScale(), true/*, pvd*/);
		if (!physics) {
			std::cerr << "ERROR: PxCreatePhysics failed\n";
			throw std::runtime_error("ERROR: PxCreatePhysics failed");
		}

		PxSceneDesc sceneDesc(physics->getTolerancesScale());
		sceneDesc.gravity = PxVec3(0.0f, -9.81f, 0.0f);
		sceneDesc.cpuDispatcher = PxDefaultCpuDispatcherCreate(1);
		sceneDesc.filterShader = PxDefaultSimulationFilterShader;
		scene = physics->createScene(sceneDesc);
		if (!scene) {
			std::cerr << "ERROR: createScene failed\n";
			throw std::runtime_error("ERROR: createScene failed");
		}

		PxMaterial* material = physics->createMaterial(0.5, 0.5, 0.25);

		glm::mat4 glmt = glm::mat4(1);
		glm::translate(glmt, glm::vec3(-1.5, 5, 0));
		glm::rotate(glmt, glm::radians<float>(180), glm::vec3(glmt[0]));
		PxTransform transform = PxTransform(PxVec3(-1.5, 5, 0));
		transform.q = PxQuat(-1, 0, 0, 0);
		cubePxActor = physics->createRigidDynamic(transform);
		{
			PxShape* shape = physics->createShape(PxBoxGeometry(0.5f, 0.5f, 0.5f), &material, 1, true);
			cubePxActor->attachShape(*shape);
			shape->release();
		}
		scene->addActor(*cubePxActor);

		PxRigidStatic* plane = physics->createRigidStatic(PxTransformFromPlaneEquation(PxPlane(PxVec3(0, 1, 0), 0)));
		{
			PxShape* shape = physics->createShape(PxPlaneGeometry(), &material, 1, true);
			plane->attachShape(*shape);
			shape->release();
		}
		scene->addActor(*plane);
	}

	void terminate() {
		physics->release();
		foundation->release();
	}
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
			res[3] = glm::vec4(glm::vec3(res[3]) + glm::vec3{ 0, -2, 0 }*dTime, 1);
			app::cam.setTransform(res);
		}
		if (up) {
			auto res = app::cam.m_transform;
			res[3] = glm::vec4(glm::vec3(res[3]) + glm::vec3{ 0, 2, 0 }*dTime, 1);
			app::cam.setTransform(res);
		}
		if (lookLeft) {
			glm::mat4 res = app::cam.m_transform;
			glm::mat4 rot = glm::rotate(glm::mat4(1), glm::radians<float>(-90 * dTime), glm::vec3{0, 1, 0});

			res[0] = rot * res[0];
			res[1] = rot * res[1];
			res[2] = rot * res[2];

			app::cam.m_transform = res;
		}
		if (lookRight) {
			glm::mat4 res = app::cam.m_transform;
			glm::mat4 rot = glm::rotate(glm::mat4(1), glm::radians<float>(90 * dTime), glm::vec3{ 0, 1, 0 });

			res[0] = rot * res[0];
			res[1] = rot * res[1];
			res[2] = rot * res[2];

			app::cam.setTransform(res);
		}
		if (lookDown) {
			app::cam.rotateX(90 * dTime);
		}
		if (lookUp) {
			app::cam.rotateX(-90 * dTime);
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
			else if (key == GLFW_KEY_ENTER) {
				glm::vec3 dir = app::cam.getTransform()[2];
				px::cubePxActor->addForce(PxVec3(dir.x*500, dir.y*500, dir.z*500));
				//px::cubePxActor->setGlobalPose(PxTransform(PxVec3(0, 5, 0)));
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

void resize(GLFWwindow* window, int width, int height) {
	app::renderer.setViewport(width, height, 0, 0);
}

int main() {
	px::init();
	app::engineBase.init();

	app::window.init();
	app::window.show();
	app::window.setKeyCallback(keybinds::keyCallback);
	app::window.setResizeCallback(resize);

	Zap::Mesh model = Zap::Mesh();
	model.load("Models/OBJ/Cube.obj");

	//Zap::Model sponzaModel = Zap::Model();
	//sponzaModel.load("Models/OBJ/Sponza.obj");

	Zap::Mesh giftModel = Zap::Mesh();
	giftModel.load("Models/OBJ/Gift.obj");

	//Actors

	Zap::Actor centre;
	centre.addMeshComponent(&model);
	for(uint32_t mc : centre.getComponents(Zap::COMPONENT_TYPE_MESH)) 
	centre.setPos(0, 0, 0);
	centre.setScale(0.5, 0.5, 0.5);

	Zap::Actor xDir;
	xDir.addMeshComponent(&model);
	xDir.setPos(1, 0, 0);
	xDir.setScale(0.5, 0.1, 0.1);

	Zap::Actor yDir;
	yDir.addMeshComponent(&model);
	yDir.setPos(0, 1, 0);
	yDir.setScale(0.1, 0.5, 0.1);

	Zap::Actor zDir;
	zDir.addMeshComponent(&model);
	zDir.setPos(0, 0, 1);
	zDir.setScale(0.1, 0.1, 0.5);

	Zap::Actor physicstest;
	physicstest.addCameraComponent({0, 0, 0});
	physicstest.addLightComponent({ 0.25, 1, 3 });

	Zap::Actor rotatingGift;
	rotatingGift.addMeshComponent(&giftModel);
	rotatingGift.setPos(3, 2, 2);

	Zap::Actor ground;
	ground.addMeshComponent(&model);
	ground.setPos(0, -1, 0);
	ground.setScale(500, 1, 500);

	Zap::Actor skybox;
	skybox.addMeshComponent(&model);
	skybox.setPos(0, 0, 0);
	skybox.setScale(500, 500, 500);

	Zap::Actor light;
	light.addLightComponent({ 2.5, 2.5, 2.5 });
	light.setPos({ -3, 2, 0 });

	Zap::Actor light2;
	light2.addLightComponent({ 3, 1.5, 0.6 });
	light2.setPos({ 3, 2, 0 });

	app::renderer.setViewport(1000, 600, 0, 0);
	app::renderer.init();

	app::renderer2.setViewport(500, 300, 0, 0);
	app::renderer2.init();

	app::cam.setPos(-1, 1, -5);
	app::cam.addCameraComponent(glm::vec3(0, 0, 0));

	//mainloop
	uint64_t currentFrame = 0;
	float dTime = 0;
	while (!app::window.shouldClose()) {
		auto timeStartFrame = std::chrono::high_resolution_clock::now();
		movement::move(dTime);
		{
			PxVec3 pos = px::cubePxActor->getGlobalPose().p;
			PxQuat orientation = px::cubePxActor->getGlobalPose().q;
			physicstest.setTransform(glm::mat4(glm::make_quat(&orientation.x)));
			physicstest.setPos(pos.x, pos.y, pos.z);
			physicstest.setScale(0.5, 0.5, 0.5);
		}

		rotatingGift.rotateY(15 * dTime);

		if (dTime > 0) {
			px::scene->simulate(dTime);
			px::scene->fetchResults(true);
		}

		app::window.clear();

		app::renderer.render(app::cam.getComponents(Zap::COMPONENT_TYPE_CAMERA)[0]);
		app::window.clearDepthStencil();
		app::renderer2.render(physicstest.getComponents(Zap::COMPONENT_TYPE_CAMERA)[0]);

		app::window.swapBuffers();
		Zap::Window::pollEvents();
		currentFrame++;
		auto timeEndFrame = std::chrono::high_resolution_clock::now();
		dTime = std::chrono::duration_cast<std::chrono::duration<float>>(timeEndFrame - timeStartFrame).count();
	}

	//terminate
	app::renderer.~PBRenderer();
	app::renderer2.~PBRenderer();
	app::window.~Window();

	app::engineBase.terminate();
	px::terminate();

#ifdef _DEBUG
	system("pause");
#endif
	return 0;
}