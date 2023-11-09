#include "Zap/Scene/PhysicsComponent.h"

namespace Zap {
    std::vector<PhysicsComponent> PhysicsComponent::all;

    PhysicsComponent::PhysicsComponent(PhysicsType type, Actor* pActor)
		: Component(pActor)
	{
		m_id = all.size();
		all.push_back(*this);

		auto glmt = m_pActor->getTransformComponent()->getTransform();
		glmt[0] = glm::normalize(glmt[0]);
		glmt[1] = glm::normalize(glmt[1]);
		glmt[2] = glm::normalize(glmt[2]);

		physx::PxTransform t = physx::PxTransform(*(physx::PxMat44*)(&glmt));
		//if (!t.isValid()) t = physx::PxTransform({ glmt[3].x, glmt[3].y , glmt[3].z });

		std::cout << t.p.x << ", " << t.p.y << ", " << t.p.z << "\n";

		auto base = Base::getBase();
		switch (type) {
		case PHYSICS_TYPE_RIGID_DYNAMIC: {
			m_pxActor = base->m_pxPhysics->createRigidDynamic(t);
			m_pxActor->userData = &all.back();
			{
				physx::PxMaterial* material = base->m_pxPhysics->createMaterial(0.5, 0.5, 0.25);
				physx::PxShape* shape = base->m_pxPhysics->createShape(physx::PxBoxGeometry(0.5f, 0.5f, 0.5f), &material, 1, true);
				((physx::PxRigidDynamic*)(m_pxActor))->attachShape(*shape);
				shape->release();
			}
			base->m_pxScene->addActor(*m_pxActor);
			break;
		}
		case PHYSICS_TYPE_RIGID_STATIC: {
			m_pxActor = base->m_pxPhysics->createRigidStatic(t);
			m_pxActor->userData = &all.back();
			{
				physx::PxMaterial* material = base->m_pxPhysics->createMaterial(0.5, 0.5, 0.25);
				physx::PxShape* shape = base->m_pxPhysics->createShape(physx::PxBoxGeometry(0.5f, 0.5f, 0.5f), &material, 1, true);
				((physx::PxRigidStatic*)(m_pxActor))->attachShape(*shape);
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