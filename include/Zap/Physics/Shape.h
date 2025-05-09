#pragma once

#include "Zap/Zap.h"
#include "Zap/Physics/Geometry.h"

#include "glm.hpp"

namespace Zap {
	class PhysicsMaterial {
	public:
		PhysicsMaterial(float staticFriction, float dynamicFriction, float restitution);
		PhysicsMaterial(physx::PxMaterial* pxMaterial);
		~PhysicsMaterial();

		void release();

		float getDynamicFriction() const;

		float getStaticFriction() const;

		float getRestitution() const;
	private:
		physx::PxMaterial* m_pxMaterial;

		friend class Shape;
	};

	class Shape
	{
	public:
		Shape() = default;
		Shape(const PhysicsGeometry& geometry, PhysicsMaterial material,
			bool isExclusive = false, glm::mat4 offsetTransform = glm::mat4(1),
			physx::PxShapeFlags shapeFlags = physx::PxShapeFlag::eVISUALIZATION | physx::PxShapeFlag::eSCENE_QUERY_SHAPE | physx::PxShapeFlag::eSIMULATION_SHAPE
		);
		Shape(physx::PxShape* pxShape);
		~Shape();

		operator physx::PxShape&() { return *getPxShape(); }
		operator physx::PxShape*() { return getPxShape(); }
 
		void release();

		void setGeometry(const PhysicsGeometry& geometry);

		// returns a copy of the geometry
		std::unique_ptr<PhysicsGeometry> getGeometry();

		bool isExclusive();

		/*
		* Sets the local pose relative to the actors transform
		* Ignores scaling, because this is handled by the underlying geometry
		*/
		void setLocalPose(glm::mat4 transform);

		void setLocalPosition(glm::vec3 pos);

		void setLocalRotation(glm::quat quat);

		glm::mat4 getLocalPose();

		glm::vec3 getLocalPosition();

		glm::quat getLocalRotation();

		PhysicsMaterial getMaterial();

		physx::PxShape* getPxShape();

	private:
		physx::PxShape* m_pxShape = nullptr;

		friend class Actor;
		friend class PhysicsComponent;
		friend class RigidBodyComponent;
		friend class RigidDynamic;
		friend class RigidStatic;
	};
}

