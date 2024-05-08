#pragma once

class physx::PxRigidDynamic;
class physx::PxRigidStatic;
namespace Zap {

	struct RigidDynamic {
		physx::PxRigidDynamic* pxActor;
	};

	struct RigidStatic {
		physx::PxRigidStatic* pxActor;
	};
}

