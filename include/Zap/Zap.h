#pragma once
#define GLM_FORCE_LEFT_HANDED
#define GLM_FORCE_QUAT_DATA_XYZW
#include "Zap/UUID.h"
#include "VulkanFramework.h"
#include "PxPhysicsAPI.h"
#include <unordered_map>

//TODO add standart renderer for windows with no renderer
namespace Zap {
	class Window;
	class Renderer;
	class Scene;

	struct SceneData {
		physx::PxScene* m_pxScene;
	};

	class Base {
	public:
		void init();

		void terminate();

		Scene createScene();

		static Base* createBase(const char* applicationName);

		static void releaseBase();

		static Base* getBase();

	private:
		Base(std::string applicationName);
		~Base();

		bool m_isInit;

		std::string m_applicationName;

		std::unordered_map<UUID, SceneData> m_scenes = {};

		//physx variables
		physx::PxFoundation* m_pxFoundation;
		physx::PxPvd* m_pxPvd;
		physx::PxPhysics* m_pxPhysics;

		static Base* m_engineBase;
		static bool m_exists;

		friend class Scene;
		friend class PhysicsComponent;
		friend class RigidBodyComponent;
		friend class RigidDynamicComponent;
		friend class RigidStaticComponent;
		friend class Shape;
		friend class PhysicsMaterial;
	};

	namespace objects {
		static std::vector<Window*> windows;// fix global variables
	}

	namespace GlobalSettings {
		VkFormat getColorFormat();
		VkFormat getDepthStencilFormat();
	}
}