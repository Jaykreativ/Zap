#pragma once

#include "Zap/Zap.h"
#include "glm.hpp"

namespace Zap {
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
		eGEOMETRY_TYPE_SPHERE = 1,
		eGEOMETRY_TYPE_CAPSULE = 2,
		eGEOMETRY_TYPE_BOX = 3,
		eGEOMETRY_TYPE_PLANE = 4,
		eGEOMETRY_TYPE_CONVEX_MESH = 5,
		eGEOMETRY_TYPE_TRIANGLE_MESH = 6,
		eGEOMETRY_TYPE_HEIGHT_FIELD = 7
	};

	class SphereGeometry : public PhysicsGeometry {
	public:
		SphereGeometry(float radius);
		SphereGeometry(const physx::PxSphereGeometry& geometry);
		SphereGeometry(SphereGeometry& geometry);

		physx::PxGeometryType::Enum getType() const override;

		physx::PxGeometry* getPxGeometry() override;
		const physx::PxGeometry* getPxGeometry() const override;

		void setRadius(float radius);

		float getRadius();

	private:
		physx::PxSphereGeometry m_geometry;
	};

	class CapsuleGeometry : public PhysicsGeometry {
	public:
		CapsuleGeometry(float radius, float halfHeight);
		CapsuleGeometry(const physx::PxCapsuleGeometry& geometry);
		CapsuleGeometry(CapsuleGeometry& geometry);

		physx::PxGeometryType::Enum getType() const override;

		physx::PxGeometry* getPxGeometry() override;
		const physx::PxGeometry* getPxGeometry() const override;

		void setRadius(float radius);

		void setHalfHeight(float halfHeight);

		float getRadius();

		float getHalfHeight();

	private:
		physx::PxCapsuleGeometry m_geometry;
	};

	class BoxGeometry : public PhysicsGeometry {
	public:
		BoxGeometry(glm::vec3 size);
		BoxGeometry(const physx::PxBoxGeometry& geometry);
		BoxGeometry(BoxGeometry& geometry);

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
		PlaneGeometry(PlaneGeometry& geometry);

		physx::PxGeometryType::Enum getType() const override;

		physx::PxGeometry* getPxGeometry() override;
		const physx::PxGeometry* getPxGeometry() const override;

	private:
		physx::PxPlaneGeometry m_geometry;
	};
}
