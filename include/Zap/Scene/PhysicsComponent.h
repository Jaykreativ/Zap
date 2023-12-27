#pragma once

#include "Zap/Zap.h"
#include "Zap/Scene/Component.h"
#include "Zap/Scene/Actor.h"
#include "Zap/Scene/Scene.h"
#include "Zap/Scene/Transform.h"

namespace Zap {
	class PhysicsComponent : public Component {
	public:
		PhysicsComponent(Actor* pActor);

	protected:
		physx::PxActor* m_pxActor;

		friend class Base;
	};// TODO add RigidActor

	class RigidBodyComponent : public PhysicsComponent {
	public:
		RigidBodyComponent(Actor* pActor, Shape shape);

		void updatePose(bool autowake = true);
	};

	class RigidDynamicComponent : public RigidBodyComponent {
	public:
		RigidDynamicComponent(Actor* pActor, Shape shape, Scene scene);

		void addForce(const glm::vec3& force);

		void clearForce();

		void addTorque(const glm::vec3& torque);

		void clearTorque();

		void wakeUp();

		void setFlag(physx::PxActorFlag::Enum flag, bool value);

		bool getFlag(physx::PxActorFlag::Enum flag);

	private:
		static std::vector<RigidDynamicComponent> all;

		friend class Base;
		friend class Scene;
		friend class Actor;
	};

	class RigidStaticComponent : public RigidBodyComponent {
	public:
		RigidStaticComponent(Actor* pActor, Shape shape, Scene scene);

	private:
		static std::vector<RigidStaticComponent> all;

		friend class Base;
		friend class Scene;
		friend class Actor;
	};
}

