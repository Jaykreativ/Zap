#pragma once

#include "Zap/Zap.h"
#include "Zap/Scene/Component.h"

namespace Zap {
	template<class T>
	class PhysicsComponent : public Component {
	public:
		PhysicsComponent(Actor* pActor)
			: Component(pActor)
		{
			m_pxActor = 
		}

	private:
		T* m_pxActor;
	};
}

