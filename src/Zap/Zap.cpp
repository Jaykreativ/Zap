#include "Zap/Zap.h"
#include "Zap/Scene/MeshComponent.h"
#include "Zap/Scene/PhysicsComponent.h"


class SimulationCallbacks : public physx::PxSimulationEventCallback {// TODO put this in scene class
	virtual void onConstraintBreak(physx::PxConstraintInfo* constraints, physx::PxU32 count) override;
	virtual void onWake(physx::PxActor** actors, physx::PxU32 count) override;
	virtual void onSleep(physx::PxActor** actors, physx::PxU32 count) override;
	virtual void onContact(const physx::PxContactPairHeader& pairHeader, const physx::PxContactPair* pairs, physx::PxU32 nbPairs) override;
	virtual void onTrigger(physx::PxTriggerPair* pairs, physx::PxU32 count) override;
	virtual void onAdvance(const physx::PxRigidBody* const* bodyBuffer, const physx::PxTransform* poseBuffer, const physx::PxU32 count) override;
};

void SimulationCallbacks::onConstraintBreak(physx::PxConstraintInfo* constraints, physx::PxU32 count) {

}
void SimulationCallbacks::onWake(physx::PxActor** actors, physx::PxU32 count) {
	std::cout << count << " :WakeCount\n";
}
void SimulationCallbacks::onSleep(physx::PxActor** actors, physx::PxU32 count) {

}
void SimulationCallbacks::onContact(const physx::PxContactPairHeader& pairHeader, const physx::PxContactPair* pairs, physx::PxU32 nbPairs) {

}
void SimulationCallbacks::onTrigger(physx::PxTriggerPair* pairs, physx::PxU32 count) {

}
void SimulationCallbacks::onAdvance(const physx::PxRigidBody* const* bodyBuffer, const physx::PxTransform* poseBuffer, const physx::PxU32 count) {

}

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
	};

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

		physx::PxSceneDesc sceneDesc(m_pxPhysics->getTolerancesScale());
		sceneDesc.gravity = physx::PxVec3(0.0f, -9.81f, 0.0f);
		sceneDesc.flags |= physx::PxSceneFlag::eENABLE_ACTIVE_ACTORS;
		sceneDesc.cpuDispatcher = physx::PxDefaultCpuDispatcherCreate(2);
		sceneDesc.filterShader = physx::PxDefaultSimulationFilterShader;
		sceneDesc.simulationEventCallback = &simulationCallbacks;
		m_pxScene = m_pxPhysics->createScene(sceneDesc);
		if (!m_pxScene) {
			std::cerr << "ERROR: createScene failed\n";
			throw std::runtime_error("ERROR: createScene failed");
		}

		physx::PxPvdSceneClient* pvdClient = m_pxScene->getScenePvdClient();
		if (pvdClient)
		{
			pvdClient->setScenePvdFlag(physx::PxPvdSceneFlag::eTRANSMIT_CONSTRAINTS, true);
			pvdClient->setScenePvdFlag(physx::PxPvdSceneFlag::eTRANSMIT_CONTACTS, true);
			pvdClient->setScenePvdFlag(physx::PxPvdSceneFlag::eTRANSMIT_SCENEQUERIES, true);
		}
	}

	void Base::terminate() {
		for (Mesh mesh : Mesh::all) mesh.destroy();

		for (PhysicsComponent& pc : RigidDynamicComponent::all) pc.m_pxActor->release();
		
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