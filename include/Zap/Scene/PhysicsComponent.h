#pragma once

#include "Zap/Zap.h"
#include "Zap/Scene/Component.h"
#include "Zap/Scene/Actor.h"
#include "Zap/Scene/Transform.h"

namespace Zap {
	enum PhysiscsType {
		RIGID_DYNAMIC = 0
	};

	class PhysicsComponent : public Component {
	public:
		PhysicsComponent(PhysiscsType type, Actor* pActor)
			: Component(pActor)
		{
			physx::PxTransform t = physx::PxTransform(*(physx::PxMat44*)(&m_pActor->getTransformComponent(0)->getTransform()));

			switch(type){
			case RIGID_DYNAMIC:
				m_pxActor = Base::getBase()->m_pxPhysics->createRigidDynamic(t);
			default:
				std::cerr << "Not yet added to constructor: " << type << "\n";
			}
		}

	private:
		PhysiscsType m_type;
		physx::PxActor* m_pxActor;
	};
}

