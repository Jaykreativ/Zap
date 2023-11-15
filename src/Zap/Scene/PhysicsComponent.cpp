#include "Zap/Scene/PhysicsComponent.h"
#include "Zap/Scene/Shape.h"

namespace Zap {
    std::vector<PhysicsComponent> PhysicsComponent::all;

    PhysicsComponent::PhysicsComponent(Actor* pActor, PhysicsType type, Shape shape)
		: Component(pActor), m_type(type)
	{
		m_id = all.size();

		auto glmt = m_pActor->getTransformComponent()->getTransform();
		glmt[0] = glm::vec4(glm::normalize(glm::vec3(glmt[0])), glmt[0].w);
		glmt[1] = glm::vec4(glm::normalize(glm::vec3(glmt[1])), glmt[1].w);
		glmt[2] = glm::vec4(glm::normalize(glm::vec3(glmt[2])), glmt[2].w);

		physx::PxTransform t = physx::PxTransform(*(physx::PxMat44*)(&glmt));

		auto base = Base::getBase();
		switch (type) {
		case PHYSICS_TYPE_RIGID_DYNAMIC: {
			m_pxActor = base->m_pxPhysics->createRigidDynamic(t);
			m_pxActor->userData = (void*)m_id;
			((physx::PxRigidDynamic*)(m_pxActor))->attachShape(*shape.m_pxShape);
			base->m_pxScene->addActor(*m_pxActor);
			break;
		}
		case PHYSICS_TYPE_RIGID_STATIC: {
			m_pxActor = base->m_pxPhysics->createRigidStatic(t);
			m_pxActor->userData = (void*)m_id;
			((physx::PxRigidDynamic*)(m_pxActor))->attachShape(*shape.m_pxShape);
			base->m_pxScene->addActor(*m_pxActor);
			break;
		}
		default:
			std::cerr << "Not yet added to constructor: " << type << "\n";
			throw std::runtime_error("Not yet added to constructor\n");
		}

		all.push_back(*this);
	}
}