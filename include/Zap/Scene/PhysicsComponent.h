#pragma once

class physx::PxRigidDynamic;
class physx::PxRigidStatic;
namespace Zap {

	struct RigidDynamicComponent {
		physx::PxRigidDynamic* pxActor;
	};

	struct RigidStaticComponent {
		physx::PxRigidStatic* pxActor;
	};
}

