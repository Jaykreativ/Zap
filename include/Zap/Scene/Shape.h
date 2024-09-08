#pragma once

#include "Zap/Zap.h"
#include "glm.hpp"

namespace Zap {
	class PhysicsMaterial {
	public:
		PhysicsMaterial(float staticFriction, float dynamicFriction, float restitution);
		~PhysicsMaterial();

		void release();

	private:
		physx::PxMaterial* m_pxMaterial;

		friend class Shape;
	};

	class PhysicsGeometry {
	public:
		PhysicsGeometry() = default;

	protected:
		physx::PxGeometry* m_pxGeometry = nullptr;

	private:

		friend class Shape;
	};

	enum PhysicsGeometryType {
		eGEOMETRY_TYPE_NONE = 0,
		eGEOMETRY_TYPE_BOX = 1,
		eGEOMETRY_TYPE_PLANE = 2
	};

	class BoxGeometry : public PhysicsGeometry {
	public:
		BoxGeometry(glm::vec3 size);
		~BoxGeometry();
		BoxGeometry(BoxGeometry& boxGeometry);
	};

	class PlaneGeometry : public PhysicsGeometry {
	public:
		PlaneGeometry();
		~PlaneGeometry();
		PlaneGeometry(PlaneGeometry& planeGeometry);
	};

	class Shape
	{
	public:
		Shape() = default;
		Shape(PhysicsGeometry& geometry, PhysicsMaterial material,
			bool isExclusive = false, glm::mat4 offsetTransform = glm::mat4(1),
			physx::PxShapeFlags shapeFlags = physx::PxShapeFlag::eVISUALIZATION | physx::PxShapeFlag::eSCENE_QUERY_SHAPE | physx::PxShapeFlag::eSIMULATION_SHAPE
		);
		Shape(physx::PxShape* pxShape);
		~Shape();
 
		void release();

		void setGeometry(PhysicsGeometry& geometry);

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

