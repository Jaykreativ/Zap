#pragma once

#include "Zap/Zap.h"

namespace Zap {
	class Actor;// declare for not having to inlude Actor.h -> "forwarded declaration"

	enum ComponentType {
		COMPONENT_TYPE_NONE = 0,
		COMPONENT_TYPE_MESH = 1,
		COMPONENT_TYPE_LIGHT = 3
	};

	class Component
	{
	public:
		Component(Actor* pActor)
			: m_pActor(pActor)
		{}

		uint32_t getID() { return m_id; }

		static ComponentType type() { return COMPONENT_TYPE_NONE; };

	protected:
		uint32_t m_id;
		Actor* m_pActor;
	};
}

