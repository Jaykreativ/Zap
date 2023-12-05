#include "Zap/Scene/PhysicsComponent.h"
#include "Zap/Scene/Shape.h"

namespace Zap {
    std::vector<PhysicsComponent> PhysicsComponent::all;

    PhysicsComponent::PhysicsComponent(Actor* pActor, Shape shape)
		: Component(pActor)
	{
		m_id = all.size();
		all.push_back(*this);
	}

	RigidBodyComponent::RigidBodyComponent(Actor* pActor, Shape shape)
		: PhysicsComponent(pActor, shape)
	{
	
	}

	RigidDynamicComponent::RigidDynamicComponent(Actor* pActor, Shape shape)
		: RigidBodyComponent(pActor, shape)
	{
		auto base = Base::getBase();
		auto 
		m_pxActor = base->m_pxPhysics->createRigidDynamic(convertGlmMat(m_pActor->getTransform()));
		m_pxActor->userData = (void*)m_id;
		((physx::PxRigidDynamic*)(m_pxActor))->attachShape(*shape.m_pxShape);
		base->m_pxScene->addActor(*m_pxActor);
	}
}

physx::PxTransform convertGlmMat(glm::mat4 glmt) {
	glmt[0] = glm::vec4(glm::normalize(glm::vec3(glmt[0])), glmt[0].w);
	glmt[1] = glm::vec4(glm::normalize(glm::vec3(glmt[1])), glmt[1].w);
	glmt[2] = glm::vec4(glm::normalize(glm::vec3(glmt[2])), glmt[2].w);

	return physx::PxTransform(*(physx::PxMat44*)(&glmt));
}

