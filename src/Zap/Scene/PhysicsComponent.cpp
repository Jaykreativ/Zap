#include "Zap/Scene/PhysicsComponent.h"
#include "Zap/Scene/Shape.h"
#include "glm/gtx/quaternion.hpp"

physx::PxTransform convertGlmMat(glm::mat4 glmt) {
	glmt[0] = glm::normalize(glmt[0]);
	glmt[1] = glm::normalize(glmt[1]);
	glmt[2] = glm::normalize(glmt[2]);

	auto pos = *((physx::PxVec3*)&glm::vec3(glmt[3]));
	auto quat = *((physx::PxQuat*)&glm::quat_cast(glm::mat3(glmt)));
 
	return physx::PxTransform(pos, quat);
}

namespace Zap {
	PhysicsComponent::PhysicsComponent(Actor* pActor)
		: Component(pActor)
	{}

	RigidBodyComponent::RigidBodyComponent(Actor* pActor, Shape shape)
		: PhysicsComponent(pActor)
	{}

	void RigidBodyComponent::updatePose(bool autowake) {
		auto t = convertGlmMat(m_pActor->getTransform());
		((physx::PxRigidBody*)m_pxActor)->setGlobalPose(t, autowake);
	}

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
		cmp->m_pxActor->setActorFlag(physx::PxActorFlag::eSEND_SLEEP_NOTIFIES, true);
		base->m_pxScene->addActor(*cmp->m_pxActor);
	}

	void RigidDynamicComponent::addForce(const glm::vec3& force) {
		((physx::PxRigidDynamic*)m_pxActor)->addForce(*((physx::PxVec3*)&force), physx::PxForceMode::eFORCE, true);
	}

	void RigidDynamicComponent::clearForce() {
		((physx::PxRigidDynamic*)m_pxActor)->clearForce();
	}

	void RigidDynamicComponent::addTorque(const glm::vec3& torque) {
		((physx::PxRigidDynamic*)m_pxActor)->addTorque(*((physx::PxVec3*)&torque), physx::PxForceMode::eFORCE, true);
	}

	void RigidDynamicComponent::clearTorque() {
		((physx::PxRigidDynamic*)m_pxActor)->clearTorque();
	}

	void RigidDynamicComponent::wakeUp() {
		((physx::PxRigidDynamic*)m_pxActor)->wakeUp();
	}

	void RigidDynamicComponent::setFlag(physx::PxActorFlag::Enum flag, bool value) { // TODO Make own enum
		((physx::PxRigidDynamic*)m_pxActor)->setActorFlag(flag, value);
	}

	bool RigidDynamicComponent::getFlag(physx::PxActorFlag::Enum flag) {
		return (flag & (uint8_t)((physx::PxRigidDynamic*)m_pxActor)->getActorFlags()) == flag;
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

