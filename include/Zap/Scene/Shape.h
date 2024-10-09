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
		~PhysicsGeometry() = default;

		operator physx::PxGeometry& () { return *getPxGeometry(); };
		operator physx::PxGeometry* () { return getPxGeometry(); };
		operator const physx::PxGeometry& () const { return *getPxGeometry(); };
		operator const physx::PxGeometry* () const { return getPxGeometry(); };

		virtual physx::PxGeometryType::Enum getType() const = 0;

		virtual physx::PxGeometry* getPxGeometry() = 0;
		virtual const physx::PxGeometry* getPxGeometry() const = 0;
	};

	enum PhysicsGeometryType {
		eGEOMETRY_TYPE_NONE = 0,
		eGEOMETRY_TYPE_BOX = 1,
		eGEOMETRY_TYPE_PLANE = 2
	};

	class BoxGeometry : public PhysicsGeometry {
	public:
		BoxGeometry(glm::vec3 size);
		BoxGeometry(const physx::PxBoxGeometry& geometry);
		BoxGeometry(BoxGeometry& boxGeometry);

		physx::PxGeometryType::Enum getType() const override;

		physx::PxGeometry* getPxGeometry() override;
		const physx::PxGeometry* getPxGeometry() const override;

		void setHalfExtents(glm::vec3 halfExtents);

		glm::vec3 getHalfExtents();

	private:
		physx::PxBoxGeometry m_geometry;
	};

	class PlaneGeometry : public PhysicsGeometry {
	public:
		PlaneGeometry();
		PlaneGeometry(const physx::PxPlaneGeometry& geometry);
		PlaneGeometry(PlaneGeometry& planeGeometry);

		physx::PxGeometryType::Enum getType() const override;

		physx::PxGeometry* getPxGeometry() override;
		const physx::PxGeometry* getPxGeometry() const override;

	private:
		physx::PxPlaneGeometry m_geometry;
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

		std::unique_ptr<PhysicsGeometry> getGeometry();

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

