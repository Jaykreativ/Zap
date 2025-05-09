#pragma once

#include "Zap/Zap.h"
#include "Zap/Physics/HitMesh.h"

#include "glm.hpp"

namespace Zap {
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

	class PhysicsGeometry {
	public:
		PhysicsGeometry() = default;
		~PhysicsGeometry() = default;

		operator physx::PxGeometry& () { return *getPxGeometry(); };
		operator physx::PxGeometry* () { return getPxGeometry(); };
		operator const physx::PxGeometry& () const { return *getPxGeometry(); };
		operator const physx::PxGeometry* () const { return getPxGeometry(); };

		virtual PhysicsGeometryType getType() const = 0;
		virtual physx::PxGeometryType::Enum getTypePx() const = 0;

		virtual physx::PxGeometry* getPxGeometry() = 0;
		virtual const physx::PxGeometry* getPxGeometry() const = 0;
	};

	class SphereGeometry : public PhysicsGeometry {
	public:
		SphereGeometry(float radius);
		SphereGeometry(const physx::PxSphereGeometry& geometry);
		SphereGeometry(SphereGeometry& geometry);

		PhysicsGeometryType getType() const override;
		physx::PxGeometryType::Enum getTypePx() const override;

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

		PhysicsGeometryType getType() const override;
		physx::PxGeometryType::Enum getTypePx() const override;

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

		PhysicsGeometryType getType() const override;
		physx::PxGeometryType::Enum getTypePx() const override;

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

		PhysicsGeometryType getType() const override;
		physx::PxGeometryType::Enum getTypePx() const override;

		physx::PxGeometry* getPxGeometry() override;
		const physx::PxGeometry* getPxGeometry() const override;

	private:
		physx::PxPlaneGeometry m_geometry;
	};

	// wrapper class for PxConvexMesh*
	class ConvexMesh {
	public:
		ConvexMesh(HitMesh hitMesh);
		ConvexMesh(physx::PxConvexMeshDesc convexDesc);
		~ConvexMesh();

		void release();

		physx::PxConvexMesh* getPxConvexMesh();

	private:
		HitMesh m_hitMesh;
		physx::PxConvexMesh* m_convexMesh;

		friend class ConvexMeshGeometry;
	};

	class ConvexMeshGeometry : public PhysicsGeometry {
	public:
		ConvexMeshGeometry(ConvexMesh& convexMesh);
		ConvexMeshGeometry(const physx::PxConvexMeshGeometry& geometry);
		ConvexMeshGeometry(ConvexMeshGeometry& geometry);

		PhysicsGeometryType getType() const override;
		physx::PxGeometryType::Enum getTypePx() const override;

		HitMesh getHitMesh();

		physx::PxGeometry* getPxGeometry() override;
		const physx::PxGeometry* getPxGeometry() const override;

	private:
		HitMesh m_hitMesh;
		physx::PxConvexMeshGeometry m_geometry;
	};
}
