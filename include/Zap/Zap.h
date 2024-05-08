#pragma once

#define ZP_ASSERT(val, str)\
			if (!val){\
				std::cerr << str << "\n";\
				throw std::runtime_error(str);\
			}

#define ZP_IS_FLAG_ENABLED(val, flag) ((val & flag) == flag)

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
	class Mesh;

	typedef vk::Image Image;

	enum Extension {
		eNONE = 0x0,
		eRAYTRACING = 0x1
	};

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

	struct Settings {
		uint32_t requestedGPU = 0;
		bool enableRaytracing = false;
	};

	class Base {
	public:
		void init();

		void update();

		void terminate();

		Settings* getSettings();

		std::string getApplicationName();

		static Base* createBase(const char* applicationName);

		static void releaseBase();

		static Base* getBase();

#ifndef ZP_ALL_PUBLIC
	private:
#endif
		Base(std::string applicationName);
		~Base();

		bool m_isInit;

		Settings m_settings = {};

		std::string m_applicationName;

		vk::Registery m_registery;

		//physx variables
		physx::PxFoundation* m_pxFoundation;
		physx::PxPvd* m_pxPvd;
		physx::PxPhysics* m_pxPhysics;

		std::vector<Zap::Mesh> m_meshes;
		vk::Sampler m_textureSampler;
		std::vector<vk::Image> m_textures = {};
		std::vector<std::string> m_texturePaths = {};

		static Base* m_engineBase;
		static bool m_exists;

		friend class Scene;
		friend class Actor;
		friend class Mesh;
		friend class PBRenderer;
		friend class RaytracingRenderer;
		friend class PathTracer;
		friend class ModelLoader;
		friend class PhysicsComponent;
		friend class RigidBodyComponent;
		friend class RigidDynamic;
		friend class RigidStatic;
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