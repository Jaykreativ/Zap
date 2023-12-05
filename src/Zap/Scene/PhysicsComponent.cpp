#include "Zap/Scene/PhysicsComponent.h"
#include "Zap/Scene/Shape.h"

physx::PxTransform convertGlmMat(glm::mat4 glmt) {
	glmt[0] = glm::vec4(glm::normalize(glm::vec3(glmt[0])), glmt[0].w);
	glmt[1] = glm::vec4(glm::normalize(glm::vec3(glmt[1])), glmt[1].w);
	glmt[2] = glm::vec4(glm::normalize(glm::vec3(glmt[2])), glmt[2].w);

	return physx::PxTransform(*(physx::PxMat44*)(&glmt));
}

namespace Zap {
	PhysicsComponent::PhysicsComponent(Actor* pActor)
		: Component(pActor)
	{}

	RigidBodyComponent::RigidBodyComponent(Actor* pActor, Shape shape)
		: PhysicsComponent(pActor)
	{}

	std::vector<RigidDynamicComponent> RigidDynamicComponent::all;

	RigidDynamicComponent::RigidDynamicComponent(Actor* pActor, Shape shape)
		: RigidBodyComponent(pActor, shape)
	{
		m_id = all.size();
		all.push_back(*this);
		auto cmp = &all.back();

		auto base = Base::getBase();
		cmp->m_pxActor = base->m_pxPhysics->createRigidDynamic(convertGlmMat(m_pActor->getTransform()));
		cmp->m_pxActor->userData = (void*)m_id;
		((physx::PxRigidDynamic*)(cmp->m_pxActor))->attachShape(*shape.m_pxShape);
		base->m_pxScene->addActor(*cmp->m_pxActor);
	}

	std::vector<RigidStaticComponent> RigidStaticComponent::all;

	RigidStaticComponent::RigidStaticComponent(Actor* pActor, Shape shape)
		: RigidBodyComponent(pActor, shape)
	{
		m_id = all.size();
		all.push_back(*this);
		auto cmp = &all.back();

		auto base = Base::getBase();
		cmp->m_pxActor = base->m_pxPhysics->createRigidStatic(convertGlmMat(m_pActor->getTransform()));
		cmp->m_pxActor->userData = (void*)m_id;
		((physx::PxRigidStatic*)(cmp->m_pxActor))->attachShape(*shape.m_pxShape);
		base->m_pxScene->addActor(*cmp->m_pxActor);
	}
}

