#pragma once

#include "Zap/Zap.h"

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
		SphereGeometry(float radius = 1);
		SphereGeometry(const physx::PxSphereGeometry& geometry);
		SphereGeometry(SphereGeometry& geometry);

		PhysicsGeometryType getType() const override;
		physx::PxGeometryType::Enum getTypePx() const override;

		physx::PxGeometry* getPxGeometry() override;
		const physx::PxGeometry* getPxGeometry() const override;

		void setRadius(float radius);

		float getRadius() const;

	private:
		physx::PxSphereGeometry m_geometry;
	};

	class CapsuleGeometry : public PhysicsGeometry {
	public:
		CapsuleGeometry(float radius = 1, float halfHeight = 1);
		CapsuleGeometry(const physx::PxCapsuleGeometry& geometry);
		CapsuleGeometry(CapsuleGeometry& geometry);

		PhysicsGeometryType getType() const override;
		physx::PxGeometryType::Enum getTypePx() const override;

		physx::PxGeometry* getPxGeometry() override;
		const physx::PxGeometry* getPxGeometry() const override;

		void setRadius(float radius);

		void setHalfHeight(float halfHeight);

		float getRadius() const;

		float getHalfHeight() const;

	private:
		physx::PxCapsuleGeometry m_geometry;
	};

	class BoxGeometry : public PhysicsGeometry {
	public:
		BoxGeometry(glm::vec3 size = {1, 1, 1});
		BoxGeometry(const physx::PxBoxGeometry& geometry);
		BoxGeometry(BoxGeometry& geometry);

		PhysicsGeometryType getType() const override;
		physx::PxGeometryType::Enum getTypePx() const override;

		physx::PxGeometry* getPxGeometry() override;
		const physx::PxGeometry* getPxGeometry() const override;

		void setHalfExtents(glm::vec3 halfExtents);

		glm::vec3 getHalfExtents() const;

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
		ConvexMesh(AssetHandle<Mesh> hitMesh);
		ConvexMesh(physx::PxConvexMeshDesc convexDesc);
		~ConvexMesh();

		void release();

		physx::PxConvexMesh* getPxConvexMesh();

	private:
		AssetHandle<Mesh> m_hitMesh;
		physx::PxConvexMesh* m_convexMesh = nullptr;

		friend class ConvexMeshGeometry;
	};

	class ConvexMeshGeometry : public PhysicsGeometry {
	public:
		ConvexMeshGeometry(){}
		ConvexMeshGeometry(ConvexMesh& convexMesh);
		ConvexMeshGeometry(const physx::PxConvexMeshGeometry& geometry, AssetHandle<Mesh> hitMesh);
		ConvexMeshGeometry(ConvexMeshGeometry& geometry);

		PhysicsGeometryType getType() const override;
		physx::PxGeometryType::Enum getTypePx() const override;

		AssetHandle<Mesh> getHitMesh() const;

		physx::PxGeometry* getPxGeometry() override;
		const physx::PxGeometry* getPxGeometry() const override;

	private:
		AssetHandle<Mesh> m_hitMesh;
		physx::PxConvexMeshGeometry m_geometry;
	};
}
