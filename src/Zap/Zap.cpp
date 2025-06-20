#include "Zap/Zap.h"
#include "Zap/Scene/Scene.h"
#include "Zap/Physics/PhysicsComponent.h"

#include "glm/gtc/quaternion.hpp"

#include "assimp/vector3.h"
#include "assimp/matrix4x4.h"

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

	Base::Base(std::string applicationName, std::filesystem::path assetLibraryPath)
		: m_applicationName(applicationName)
	{
		if (assetLibraryPath != "")
			m_assetHandler = AssetHandler(assetLibraryPath);
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
			throw std::runtime_error("Can't initialize GLFW");

		vk::initInfo initInfo{};// init Instance
		initInfo.applicationName = m_applicationName.c_str();
		initInfo.requestedInstanceLayers = {
#if _DEBUG
			"VK_LAYER_KHRONOS_validation"
#endif
		};

		vk::initInstance(initInfo);

		// init device
		initInfo.checkDeviceSupport = false;// request extensions, layers, features
		initInfo.requestedDeviceExtensions = {
			VK_KHR_SWAPCHAIN_EXTENSION_NAME
		};

		if (m_settings.enableRaytracing) {
			initInfo.requestedDeviceExtensions.push_back(VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME);
			initInfo.requestedDeviceExtensions.push_back(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME);
			initInfo.requestedDeviceExtensions.push_back(VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME);
		}

		// add Features
		VkPhysicalDeviceRobustness2FeaturesEXT robustness2Features{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ROBUSTNESS_2_FEATURES_EXT };
		robustness2Features.nullDescriptor = true;

		VkPhysicalDeviceVulkan12Features vulkan12Features{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES };
		vulkan12Features.runtimeDescriptorArray = true;
		vulkan12Features.bufferDeviceAddress = true;
		vulkan12Features.pNext = &robustness2Features;

		VkPhysicalDeviceAccelerationStructureFeaturesKHR accelerationStructureFeatures{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_FEATURES_KHR };
		accelerationStructureFeatures.pNext = &vulkan12Features;
		accelerationStructureFeatures.accelerationStructure = true;

		VkPhysicalDeviceRayTracingPipelineFeaturesKHR raytracingPipelineFeatures{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_FEATURES_KHR };
		raytracingPipelineFeatures.pNext = &accelerationStructureFeatures;
		raytracingPipelineFeatures.rayTracingPipeline = true;

		VkPhysicalDeviceFeatures2 features2{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2 };
		if (m_settings.enableRaytracing) {
			features2.pNext = &raytracingPipelineFeatures;
			features2.features.shaderInt64 = true;
		}
		else {
			features2.pNext = &vulkan12Features;
		}

		initInfo.features = features2;

		bool requestedGPUSupported = false;
		auto physicalDevices = vk::PhysicalDevice::getAllPhysicalDevices();// validation
		uint32_t selectedDeviceIndex = 0xFFFFFFFF;
		auto printSupport = [](bool b) {
			if (b)
				std::cout << "Supported\n";
			else
				std::cout << "Not Supported\n";
		};
		uint32_t physicalDeviceIndex = 0;
		for (auto physicalDevice : physicalDevices) {
			std::cout << 
				"---------------- Device ----------------\n" << 
				physicalDevice.getName() << "\n";

			std::cout <<
				"-------------- Extensions --------------\n";
			bool allExtensionsSupported = true;
			for (auto extension : initInfo.requestedDeviceExtensions) {
				bool supported = physicalDevice.isExtensionSupported(extension);
				allExtensionsSupported &= supported;
				std::cout << extension << " | ";
				printSupport(supported);
			}

			std::cout << 
				"--------------- Features ---------------\n";
			bool allFeaturesSupported = true;
			auto supportedVk12Features = physicalDevice.getSupportedVulkan12Features();
			allFeaturesSupported &= supportedVk12Features.runtimeDescriptorArray;
			std::cout << "supportedVk12Features.runtimeDescriptorArray | ";
			printSupport(supportedVk12Features.runtimeDescriptorArray);
			allFeaturesSupported &= supportedVk12Features.bufferDeviceAddress;
			std::cout << "supportedVk12Features.bufferDeviceAddress | ";
			printSupport(supportedVk12Features.bufferDeviceAddress);
			if (m_settings.enableRaytracing) {
				auto supportedFeatures2 = physicalDevice.getSupportedFeatures2();
				auto supportedASFeatures = physicalDevice.getSupportedAccelerationStructureFeatures();
				auto supportedRTPipelineFeatures = physicalDevice.getSupportedRayTraycingPipelineFeatures();
				allFeaturesSupported &= supportedFeatures2.features.shaderInt64;
				std::cout << "supportedFeatures2.features.shaderInt64 | ";
				printSupport(supportedFeatures2.features.shaderInt64);
				allFeaturesSupported &= supportedASFeatures.accelerationStructure;
				std::cout << "supportedASFeatures.accelerationStructure | ";
				printSupport(supportedASFeatures.accelerationStructure);
				allFeaturesSupported &= supportedRTPipelineFeatures.rayTracingPipeline;
				std::cout << "supportedRTPipelineFeatures.rayTracingPipeline | ";
				printSupport(supportedRTPipelineFeatures.rayTracingPipeline);
			}

			if (allExtensionsSupported && allFeaturesSupported) {
				selectedDeviceIndex = physicalDeviceIndex;
				if (m_settings.requestedGPU == physicalDeviceIndex) {
					requestedGPUSupported = true;
					initInfo.deviceIndex = m_settings.requestedGPU;
				}
			}

			physicalDeviceIndex++;
		}
		
		ZP_ASSERT(selectedDeviceIndex == 0xFFFFFFFF, "No supported device found");

		if (!requestedGPUSupported)
			initInfo.deviceIndex = selectedDeviceIndex;

		initVulkan(initInfo);

		m_registery = vk::Registery();

		m_textureSampler = vk::Sampler();
		m_textureSampler.init();

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

		if (!m_assetHandler.getAssetLibrary().empty()) {
			m_assetHandler.loadFromFile();
		}
	}

	void Base::update() { // TODO implement base update
		return;
	}

	void Base::terminate() {
		m_assetHandler.destroyAssets();

		m_pxPhysics->release();
		m_pxFoundation->release();

		m_textureSampler.destroy();

		terminateVulkan();
		glfwTerminate();
	}

	Settings* Base::getSettings() {
		return &m_settings;
	}

	AssetHandler* Base::getAssetHandler() {
		return &m_assetHandler;
	}

	std::string Base::getApplicationName() {
		return m_applicationName;
	}

	Base* Base::createBase(std::string applicationName, std::filesystem::path assetLibraryPath) {
		m_engineBase = new Base(applicationName, assetLibraryPath);
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

	namespace PxUtils {
		physx::PxTransform glmMat4ToTransform(glm::mat4 glmt) {
			glmt[0] = glm::normalize(glmt[0]);
			glmt[1] = glm::normalize(glmt[1]);
			glmt[2] = glm::normalize(glmt[2]);

			auto pos = *((physx::PxVec3*)&glm::vec3(glmt[3]));
			auto quat = *((physx::PxQuat*)&glm::quat_cast(glm::mat3(glmt)));

			return physx::PxTransform(pos, quat);
		}

		physx::PxVec2 glmVec2toVec2(glm::vec2 vec) {
			return { vec.x, vec.y };
		}

		physx::PxVec3 glmVec3toVec3(glm::vec3 vec) {
			return { vec.x, vec.y, vec.z };
		}

		physx::PxVec4 glmVec4toVec4(glm::vec4 vec) {
			return { vec.x, vec.y, vec.z, vec.w };
		}

		glm::mat4 transformToGlmMat4(physx::PxTransform transform) {
			physx::PxMat44 pxMat4(transform);
			glm::mat4 mat4;
			mat4[0] = vec4ToGlmVec4(pxMat4.column0);
			mat4[1] = vec4ToGlmVec4(pxMat4.column1);
			mat4[2] = vec4ToGlmVec4(pxMat4.column2);
			mat4[3] = vec4ToGlmVec4(pxMat4.column3);
			return mat4;
		}

		glm::vec2 vec2ToGlmVec2(physx::PxVec2 vec) {
			return { vec.x, vec.y };
		}

		glm::vec3 vec3ToGlmVec3(physx::PxVec3 vec) {
			return { vec.x, vec.y, vec.z };
		}

		glm::vec4 vec4ToGlmVec4(physx::PxVec4 vec) {
			return { vec.x, vec.y, vec.z, vec.w };
		}

		physx::PxQuat glmQuatToQuat(glm::quat quat) {
			return physx::PxQuat(quat.x, quat.y, quat.z, quat.w);
		}

		glm::quat quatToGlmQuat(physx::PxQuat quat) {
			return glm::quat(quat.x, quat.y, quat.z, quat.w);
		}
	}

	namespace AssimpUtils {
		glm::mat4 mat4ToGlmMat4(const aiMatrix4x4& from) {
			glm::mat4 to;
			//the a,b,c,d in assimp is the row ; the 1,2,3,4 is the column
			to[0][0] = from.a1; to[1][0] = from.a2; to[2][0] = from.a3; to[3][0] = from.a4;
			to[0][1] = from.b1; to[1][1] = from.b2; to[2][1] = from.b3; to[3][1] = from.b4;
			to[0][2] = from.c1; to[1][2] = from.c2; to[2][2] = from.c3; to[3][2] = from.c4;
			to[0][3] = from.d1; to[1][3] = from.d2; to[2][3] = from.d3; to[3][3] = from.d4;
			return to;
		}

		glm::vec3 vec3ToGlmVec3(const aiVector3D& vec) {
			return glm::vec3(vec.x, vec.y, vec.z);
		}
	}
}