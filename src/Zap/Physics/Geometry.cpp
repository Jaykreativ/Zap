#include "Zap/Physics/Geometry.h"

namespace Zap {
	/* Sphere */

	SphereGeometry::SphereGeometry()
		: m_geometry()
	{}

	SphereGeometry::SphereGeometry(const physx::PxSphereGeometry& geometry)
		: m_geometry(geometry)
	{}

	SphereGeometry::SphereGeometry(SphereGeometry& planeGeometry)
		: m_geometry(planeGeometry.m_geometry)
	{}

	physx::PxGeometryType::Enum SphereGeometry::getType() const {
		return m_geometry.getType();
	}

	physx::PxGeometry* SphereGeometry::getPxGeometry() {
		return &m_geometry;
	}
	const physx::PxGeometry* SphereGeometry::getPxGeometry() const {
		return &m_geometry;
	}

	void SphereGeometry::setRadius(float radius) {
		m_geometry.radius = radius;
	}

	float SphereGeometry::getRadius() {
		return m_geometry.radius;
	}

	/* Capsule */

	CapsuleGeometry::CapsuleGeometry()
		: m_geometry()
	{}

	CapsuleGeometry::CapsuleGeometry(const physx::PxCapsuleGeometry& geometry)
		: m_geometry(geometry)
	{}

	CapsuleGeometry::CapsuleGeometry(CapsuleGeometry& planeGeometry)
		: m_geometry(planeGeometry.m_geometry)
	{}

	physx::PxGeometryType::Enum CapsuleGeometry::getType() const {
		return m_geometry.getType();
	}

	physx::PxGeometry* CapsuleGeometry::getPxGeometry() {
		return &m_geometry;
	}
	const physx::PxGeometry* CapsuleGeometry::getPxGeometry() const {
		return &m_geometry;
	}

	void CapsuleGeometry::setRadius(float radius) {
		m_geometry.radius = radius;
	}

	void CapsuleGeometry::setHalfHeight(float halfHeight) {
		m_geometry.halfHeight = halfHeight;
	}

	float CapsuleGeometry::getRadius() {
		return m_geometry.radius;
	}

	float CapsuleGeometry::getHalfHeight() {
		return m_geometry.halfHeight;
	}

	/* Box */

	BoxGeometry::BoxGeometry(glm::vec3 size) {
		m_geometry = physx::PxBoxGeometry(size.x, size.y, size.z);
	}

	BoxGeometry::BoxGeometry(const physx::PxBoxGeometry& geometry) {
		m_geometry = geometry;
	}

	BoxGeometry::BoxGeometry(BoxGeometry& boxGeometry) {
		m_geometry = boxGeometry.m_geometry;
	}

	physx::PxGeometryType::Enum BoxGeometry::getType() const {
		return m_geometry.getType();
	}

	physx::PxGeometry* BoxGeometry::getPxGeometry() {
		return &m_geometry;
	}
	const physx::PxGeometry* BoxGeometry::getPxGeometry() const {
		return &m_geometry;
	}

	void BoxGeometry::setHalfExtents(glm::vec3 halfExtents) {
		m_geometry.halfExtents = { halfExtents.x, halfExtents.y, halfExtents.z };
	}

	glm::vec3 BoxGeometry::getHalfExtents() {
		return { m_geometry.halfExtents.x, m_geometry.halfExtents.y, m_geometry.halfExtents.z };
	}

	/* Plane */

	PlaneGeometry::PlaneGeometry()
		: m_geometry()
	{}

	PlaneGeometry::PlaneGeometry(const physx::PxPlaneGeometry& geometry)
		: m_geometry(geometry)
	{}

	PlaneGeometry::PlaneGeometry(PlaneGeometry& planeGeometry)
		: m_geometry(planeGeometry.m_geometry)
	{}

	physx::PxGeometryType::Enum PlaneGeometry::getType() const {
		return m_geometry.getType();
	}

	physx::PxGeometry* PlaneGeometry::getPxGeometry() {
		return &m_geometry;
	}
	const physx::PxGeometry* PlaneGeometry::getPxGeometry() const {
		return &m_geometry;
	}

}