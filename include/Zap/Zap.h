#pragma once

#define ZP_ASSERT(val, str)\
			if (!val){\
				std::cerr << str << "\n";\
				throw std::runtime_error(str);\
			} 

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

	enum PhysicsType {
		PHYSICS_TYPE_UNDEFINED = 0,
		PHYSICS_TYPE_NONE = 1,
		PHYSICS_TYPE_RIGID_DYNAMIC = 2,
		PHYSICS_TYPE_RIGID_STATIC = 3,
		PHYSICS_TYPE_RIGID_BODY = 4
	};

	enum ComponentType {
		COMPONENT_TYPE_NONE = 0,
		COMPONENT_TYPE_TRANSFORM = 1,
		COMPONENT_TYPE_MESH = 2,
		COMPONENT_TYPE_RIGID_DYNAMIC = 3,
		COMPONENT_TYPE_RIGID_STATIC = 4,
		COMPONENT_TYPE_LIGHT = 5,
		COMPONENT_TYPE_CAMERA = 6
	};

	class Base {
	public:
		void init();

		void terminate();

		static Base* createBase(const char* applicationName);

		static void releaseBase();

		static Base* getBase();

	private:
		Base(std::string applicationName);
		~Base();

		bool m_isInit;

		std::string m_applicationName;

		//physx variables
		physx::PxFoundation* m_pxFoundation;
		physx::PxPvd* m_pxPvd;
		physx::PxPhysics* m_pxPhysics;

		static Base* m_engineBase;
		static bool m_exists;

		friend class Scene;
		friend class Actor;
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