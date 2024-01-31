#include "Zap/Zap.h"
#include "Zap/Scene/Scene.h"
#include "Zap/Scene/PhysicsComponent.h"

class SimulationCallbacks : public physx::PxSimulationEventCallback {// TODO put this in scene class
	void onConstraintBreak(physx::PxConstraintInfo* constraints, physx::PxU32 count) {
		PX_UNUSED(constraints); PX_UNUSED(count);
	}

	void onWake(physx::PxActor** actors, physx::PxU32 count) {
		PX_UNUSED(actors); PX_UNUSED(count);
	}

	void onSleep(physx::PxActor** actors, physx::PxU32 count) {
		PX_UNUSED(actors); PX_UNUSED(count);
	}

	void onContact(const physx::PxContactPairHeader& pairHeader, const physx::PxContactPair* pairs, physx::PxU32 nbPairs) {
		PX_UNUSED(pairHeader); PX_UNUSED(pairs); PX_UNUSED(nbPairs);
	}

	void onTrigger(physx::PxTriggerPair* pairs, physx::PxU32 count) {
		PX_UNUSED(pairs); PX_UNUSED(count);
	}

	void onAdvance(const physx::PxRigidBody* const* bodyBuffer, const physx::PxTransform* poseBuffer, const physx::PxU32 count) {
		PX_UNUSED(bodyBuffer); PX_UNUSED(poseBuffer); PX_UNUSED(count);
	}
};

static SimulationCallbacks simulationCallbacks = SimulationCallbacks();

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

	Base::Base(std::string applicationName) {

		m_applicationName = applicationName;
	}

	Base::~Base() {}

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

		vk::initInfo initInfo = { m_applicationName.c_str(), 0};
		initVulkan(initInfo);
		printStats();

		//init physx
		m_pxFoundation = PxCreateFoundation(PX_PHYSICS_VERSION, gDefaultAllocator, gDefaultErrorCallback);
		if (!m_pxFoundation) {
			std::cerr << "ERROR: PxCreateFoundation failed\n";
			throw std::runtime_error("ERROR: PxCreateFoundation failed");
		}

		m_pxPvd = physx::PxCreatePvd(*m_pxFoundation);
		physx::PxPvdTransport* transport = physx::PxDefaultPvdSocketTransportCreate("127.0.0.1", 5425, 10);
		m_pxPvd->connect(*transport, physx::PxPvdInstrumentationFlag::eALL);

		m_pxPhysics = PxCreatePhysics(PX_PHYSICS_VERSION, *m_pxFoundation, physx::PxTolerancesScale(), true, m_pxPvd);
		if (!m_pxPhysics) {
			std::cerr << "ERROR: PxCreatePhysics failed\n";
			throw std::runtime_error("ERROR: PxCreatePhysics failed");
		}
	}

	void Base::terminate() {
		for (Mesh mesh : Mesh::all) mesh.destroy();
		
		m_pxPhysics->release();
		m_pxFoundation->release();

		terminateVulkan();
		glfwTerminate();
	}

	Base* Base::createBase(const char* applicationName) {
		m_engineBase = new Base(applicationName);
		m_exists = true;
		return m_engineBase;
	}

	void Base::releaseBase() {
		delete m_engineBase;
		m_exists = false;
	}

	Base* Base::getBase() {
		return m_engineBase;
	}

	Base* Base::m_engineBase;
	bool Base::m_exists;
}