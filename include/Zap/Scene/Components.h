#pragma once

#include "Zap/Zap.h"
#include "Zap/AssetHandling/Asset.h"
#include "Zap/AssetHandling/AssetTypes/Mesh.h"
#include "Zap/AssetHandling/AssetTypes/Material.h"

#include "glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

namespace Zap {
	struct Name {
		Name(std::string name = "") : name(name) {}
		std::string name;
	};
	
	struct Transform {
		glm::mat4 transform = glm::mat4(1);
	};

	struct Camera {
		bool lookAtCenter = false;
		glm::mat4 offset;
	};

	struct Light
	{
		glm::vec3 color = { 1, 1, 1 };
		float strength = 1;
		float radius = 1;
	};

	struct Model {
		std::vector<AssetHandle<Mesh>> meshes;
		std::vector<AssetHandle<Material>> materials;
		std::vector<glm::mat4> transforms;
		glm::vec3 boundMin = { 0, 0, 0 };
		glm::vec3 boundMax = { 0, 0, 0 };
	};

	class physx::PxRigidDynamic;
	struct RigidDynamic {
		physx::PxRigidDynamic* pxActor;
	};

	class physx::PxRigidStatic;
	struct RigidStatic {
		physx::PxRigidStatic* pxActor;
	};
}