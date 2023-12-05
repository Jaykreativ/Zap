#pragma once

#include "Zap/Zap.h"
#include "Zap/Scene/Component.h"
#include "Zap/Scene/Actor.h"
#include "Zap/Scene/Transform.h"

namespace Zap {

	class PhysicsComponent : public Component {
	public:
		PhysicsComponent(Actor* pActor);

	protected:
		physx::PxActor* m_pxActor;

		friend class Base;
	};

	class RigidBodyComponent : public PhysicsComponent {
	public:
		RigidBodyComponent(Actor* pActor, Shape shape);
	};

	class RigidDynamicComponent : public RigidBodyComponent {
	public:
		RigidDynamicComponent(Actor* pActor, Shape shape);

	private:
		static std::vector<RigidDynamicComponent> all;

		friend class Base;
		friend class Scene;
	};

	class RigidStaticComponent : public RigidBodyComponent {
	public:
		RigidStaticComponent(Actor* pActor, Shape shape);

	private:
		static std::vector<RigidStaticComponent> all;

		friend class Base;
	};
}

