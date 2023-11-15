#pragma once

#include "Zap/Zap.h"
#include "Zap/Scene/Component.h"
#include "Zap/Scene/Actor.h"
#include "Zap/Scene/Transform.h"

namespace Zap {

	class PhysicsComponent : public Component {
	public:
		PhysicsComponent(Actor* pActor, PhysicsType type, Shape shape);

	private:
		PhysicsType m_type;
		physx::PxActor* m_pxActor;

		static std::vector<PhysicsComponent> all;

		friend class Base;
		friend class Scene;
		friend class Actor;
	};
}

