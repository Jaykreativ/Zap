#include "Zap/Scene/PhysicsComponent.h"

namespace Zap {
    std::vector<PhysicsComponent> PhysicsComponent::all;

    PhysicsComponent::PhysicsComponent(PhysicsType type, Actor* pActor)
		: Component(pActor)
	{
		physx::PxTransform t = physx::PxTransform(*(physx::PxMat44*)(&m_pActor->getTransformComponent()->getTransform()));

		switch (type) {
		case PHYSICS_TYPE_RIGID_DYNAMIC: {
			auto base = Base::getBase();
			m_pxActor = base->m_pxPhysics->createRigidDynamic(t);
			m_pxActor->userData = this;
			{
				physx::PxMaterial* material = base->m_pxPhysics->createMaterial(0.5, 0.5, 0.25);
				physx::PxShape* shape = base->m_pxPhysics->createShape(physx::PxBoxGeometry(0.5f, 0.5f, 0.5f), &material, 1, true);
				((physx::PxRigidDynamic*)(m_pxActor))->attachShape(*shape);
				shape->release();
			}
			base->m_pxScene->addActor(*m_pxActor);
			break;
		}
		default:
			std::cerr << "Not yet added to constructor: " << type << "\n";
			throw std::runtime_error("Not yet added to constructor\n");
		}
	}
}