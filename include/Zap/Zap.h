#pragma once

#define ZP_ASSERT(val, str)\
			if (!val){\
				std::cerr << "ERROR: " << str << "\n";\
				throw std::runtime_error(str);\
			}

#define ZP_WARN(val, str)\
			if (!val){\
				std::cerr << "WARNING: " << str << "\n";\
			}

#define ZP_IS_FLAG_ENABLED(val, flag) ((val & flag) == flag)

#define GLM_ENABLE_EXPERIMENTAL
#define GLM_FORCE_LEFT_HANDED
#define GLM_FORCE_QUAT_DATA_XYZW
#include "Zap/UUID.h"
#include "Zap/AssetHandler.h"
#include "VulkanFramework.h"
#define PX_PHYSX_STATIC_LIB

#include "PxPhysicsAPI.h"

#include "assimp/vector3.h"
#include "assimp/matrix4x4.h"

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

		AssetHandler* getAssetHandler();

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
		
		std::string m_assetDir = "./";
		AssetHandler m_assetHandler;
		vk::Sampler m_textureSampler;
		std::unordered_map<UUID, uint32_t> m_textureIndices = {};

		static Base* m_engineBase;
		static bool m_exists;

		friend class Scene;
		friend class Actor;
		friend class Mesh;
		friend class Material;
		friend class Texture;
		friend class RenderTaskTemplate;
		friend class PBRenderer;
		friend class RaytracingRenderer;
		friend class PathTracer;
		friend class TextureLoader;
		friend class MaterialLoader;
		friend class MeshLoader;
		friend class ModelLoader;
		friend class PhysicsComponent;
		friend class RigidBodyComponent;
		friend class RigidDynamic;
		friend class RigidStatic;
		friend class Shape;
		friend class PhysicsMaterial;
	};

	namespace GlobalSettings {
		VkFormat getColorFormat();
		VkFormat getDepthStencilFormat();
	}

	namespace PxUtils {
		/*
		* Return a PxTransform consisting of a position vector and a rotation quaternion, ingores scale
		*/
		physx::PxTransform glmMat4ToTransform(glm::mat4 glmt);

		physx::PxVec2 glmVec2toVec2(glm::vec2 vec);

		physx::PxVec3 glmVec3toVec3(glm::vec3 vec);

		physx::PxVec4 glmVec4toVec4(glm::vec4 vec);

		glm::mat4 transformToGlmMat4(physx::PxTransform transform);

		glm::vec2 vec2ToGlmVec2(physx::PxVec2 vec);

		glm::vec3 vec3ToGlmVec3(physx::PxVec3 vec);

		glm::vec4 vec4ToGlmVec4(physx::PxVec4 vec);

		physx::PxQuat glmQuatToQuat(glm::quat quat);

		glm::quat quatToGlmQuat(physx::PxQuat quat);
	}

	namespace AssimpUtils {
		glm::mat4 mat4ToGlmMat4(const aiMatrix4x4& from);

	}
}