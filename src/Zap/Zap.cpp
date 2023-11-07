#include "Zap/Zap.h"
#include "Zap/Scene/MeshComponent.h"

namespace Zap {
	namespace GlobalSettings {
		VkFormat colorFormat;
		VkFormat getColorFormat() {
			return colorFormat;
		}

		VkFormat depthStencilFormat;
		VkFormat getDepthStencilFormat() {
			return depthStencilFormat;
		}
	}

	Base::Base(const char* applicationName) {
		char* str = new char[strlen(applicationName) + 1];
		strcpy(str, applicationName);

		m_applicationName = str;
	};

	Base::~Base() {
		delete[] m_applicationName;
	}

	//physx defaults
	physx::PxDefaultAllocator gDefaultAllocator;
	physx::PxDefaultErrorCallback gDefaultErrorCallback;
	physx::PxSimulationFilterShader gDefaultFilterShader;

	void Base::init()
	{
		GlobalSettings::colorFormat = VK_FORMAT_B8G8R8A8_UNORM;
		GlobalSettings::depthStencilFormat = VK_FORMAT_D32_SFLOAT_S8_UINT;

		if (!glfwInit())
			std::runtime_error("Can't initialize GLFW");

		vk::initInfo initInfo = { m_applicationName, 1 };
		initVulkan(initInfo);
	}
		initVulkan(m_applicationName);

		//init physx
		m_pxFoundation = PxCreateFoundation(PX_PHYSICS_VERSION, gDefaultAllocator, gDefaultErrorCallback);
		if (!m_pxFoundation) {
			std::cerr << "ERROR: PxCreateFoundation failed\n";
			throw std::runtime_error("ERROR: PxCreateFoundation failed");
		}

		/*pvd = PxCreatePvd(*foundation);
		PxPvdTransport* transport = PxDefaultPvdSocketTransportCreate("127.0.0.1", 5425, 10);
		pvd->connect(*transport, PxPvdInstrumentationFlag::eALL);*/

		m_pxPhysics = PxCreatePhysics(PX_PHYSICS_VERSION, *m_pxFoundation, physx::PxTolerancesScale(), true/*, pvd*/);
		if (!m_pxPhysics) {
			std::cerr << "ERROR: PxCreatePhysics failed\n";
			throw std::runtime_error("ERROR: PxCreatePhysics failed");
		}

		physx::PxSceneDesc sceneDesc(m_pxPhysics->getTolerancesScale());
		sceneDesc.gravity = physx::PxVec3(0.0f, -9.81f, 0.0f);
		sceneDesc.cpuDispatcher = physx::PxDefaultCpuDispatcherCreate(1);
		sceneDesc.filterShader = physx::PxDefaultSimulationFilterShader;
		m_pxScene = m_pxPhysics->createScene(sceneDesc);
		if (!m_pxScene) {
			std::cerr << "ERROR: createScene failed\n";
			throw std::runtime_error("ERROR: createScene failed");
		}
	}

	void Base::terminate() {
		for (MeshComponent mc : MeshComponent::all) mc.m_pMesh->~Mesh();

		m_pxPhysics->release();
		m_pxFoundation->release();

		terminateVulkan();
		glfwTerminate();
	}

	Base* Base::createBase(const char* applicationName) {
		m_engineBase = Base(applicationName);
		m_exists = true;
		return &m_engineBase;
	}

	void Base::releaseBase() {
		m_engineBase.~Base();
		m_exists = false;
	}

	Base* Base::getBase() {
		return &m_engineBase;
	}

	Base Base::m_engineBase = Base("");
	bool Base::m_exists;
}