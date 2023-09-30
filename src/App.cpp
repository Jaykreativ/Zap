#include "Zap.h"
#include "Window.h"
#include "PBRenderer.h"
#include "PxPhysicsAPI.h"
#include "glm.hpp"
#include "gtc/matrix_transform.hpp"
#include "gtc/type_ptr.hpp"

namespace app {
	Zap::Window window = Zap::Window(1000, 600, "Zap Application");

	Zap::PBRenderer renderer = Zap::PBRenderer(window);

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
		PxTransform transform = PxTransform(PxVec3(-1.5, 5, 0));
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

int main() {
	px::init();
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

	Zap::Model objModel = Zap::Model();
	objModel.load("Models/OBJ/Cube.obj");

	//Actors
	Zap::VisibleActor centre;
	centre.setModel(model);
	centre.setPos(0, 0, 0);
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

	Zap::VisibleActor cube;
	{
		cube.setModel(model);
		PxVec3 pos = px::cubePxActor->getGlobalPose().p;
		cube.setPos(pos.x, pos.y, pos.z);
		cube.m_color = {0.5, 0.7, 1};
	}

	Zap::VisibleActor ground;
	{
		ground.setModel(model);
		ground.setTransform(glm::scale(glm::translate(glm::mat4(1), glm::vec3(0, -1, 0)), glm::vec3(500, 2, 500)));
		ground.m_color = { 0.6, 0.7, 0.5 };
	}

	app::renderer.addActor(cube);
	app::renderer.addActor(ground);

	Zap::Light light;
	light.setColor({2, 2, 2});
	light.setPos({-3, 2, 0});

	Zap::Light light2;
	light2.setColor({ 1, 0.5, 0.2 });
	light2.setPos({ 3, 2, 0 });

	app::renderer.addLight(&light2);
	app::renderer.addLight(&light);

	app::renderer.setViewport(1000, 600, 0, 0);
	app::renderer.init();

	app::cam.setPos(-1, 1, -5);

	//mainloop
	uint64_t currentFrame = 0;
	float dTime = 0;
	while (!app::window.shouldClose()) {
		auto timeStartFrame = std::chrono::high_resolution_clock::now();
		movement::move(dTime);
		{
			PxVec3 pos = px::cubePxActor->getGlobalPose().p;
			PxQuat orientation = px::cubePxActor->getGlobalPose().q;
			cube.setTransform(glm::mat4(glm::make_quat(&orientation.x)));
			cube.setPos(pos.x, pos.y, pos.z);
		}

		if (dTime > 0) {
			px::scene->simulate(dTime);
			px::scene->fetchResults(true);
		}

		app::renderer.render(&app::cam);

		app::window.swapBuffers();
		Zap::Window::pollEvents();
		currentFrame++;
		auto timeEndFrame = std::chrono::high_resolution_clock::now();
		dTime = std::chrono::duration_cast<std::chrono::duration<float>>(timeEndFrame - timeStartFrame).count();
	}

	//terminate
	app::renderer.~PBRenderer();
	app::window.~Window();

	Zap::terminate();
	px::terminate();

#ifdef _DEBUG
	system("pause");
#endif
	return 0;
}