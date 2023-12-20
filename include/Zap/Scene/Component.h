#pragma once

#include "Zap/Zap.h"

namespace Zap {
	class Actor;// declare for not having to inlude Actor.h -> "forwarded declaration"

	enum PhysicsType {
		PHYSICS_TYPE_UNDEFINED = 0,
		PHYSICS_TYPE_NONE = 1,
		PHYSICS_TYPE_RIGID_DYNAMIC = 2,
		PHYSICS_TYPE_RIGID_STATIC = 3,
		PHYSICS_TYPE_RIGID_BODY = 4
	};

	enum ComponentType {
		COMPONENT_TYPE_NONE = 0,
		COMPONENT_TYPE_TRANSFORM = 1,
		COMPONENT_TYPE_MESH = 2,
		COMPONENT_TYPE_RIGID_DYNAMIC = 3,
		COMPONENT_TYPE_RIGID_STATIC = 4,
		COMPONENT_TYPE_LIGHT = 5,
		COMPONENT_TYPE_CAMERA = 6
	};

	struct ComponentAccess
	{
		ComponentType type;
		uint32_t id;
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

		friend class Scene;
	};
}

