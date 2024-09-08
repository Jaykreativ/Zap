#include "Zap/Scene/Shape.h"

namespace Zap {
	PhysicsMaterial::PhysicsMaterial(float staticFriction, float dynamicFriction, float restitution) {
		auto base = Base::getBase();
		m_pxMaterial = base->m_pxPhysics->createMaterial(staticFriction, dynamicFriction, restitution);
	}

	PhysicsMaterial::~PhysicsMaterial() {}

	void PhysicsMaterial::release() {
		m_pxMaterial->release();
	}

	BoxGeometry::BoxGeometry(glm::vec3 size) {
		m_pxGeometry = new physx::PxBoxGeometry(size.x, size.y, size.z);
	}

	BoxGeometry::~BoxGeometry() {
		delete m_pxGeometry;
	}

	BoxGeometry::BoxGeometry(BoxGeometry& boxGeometry) {
		auto* pGeometry = (physx::PxBoxGeometry*)boxGeometry.m_pxGeometry;
		m_pxGeometry = new physx::PxBoxGeometry(pGeometry->halfExtents.x, pGeometry->halfExtents.y, pGeometry->halfExtents.z);
	}

	PlaneGeometry::PlaneGeometry() {
		m_pxGeometry = new physx::PxPlaneGeometry();
	}

	PlaneGeometry::~PlaneGeometry() {
		delete m_pxGeometry;
	}

	PlaneGeometry::PlaneGeometry(PlaneGeometry& planeGeometry) {
		m_pxGeometry = new physx::PxPlaneGeometry();
	}

	Shape::Shape(PhysicsGeometry& geometry, PhysicsMaterial material, bool isExclusive, glm::mat4 offsetTransform, physx::PxShapeFlags shapeFlags) {
		auto base = Base::getBase();

		m_pxShape = base->m_pxPhysics->createShape(*geometry.m_pxGeometry, &material.m_pxMaterial, 1, isExclusive, shapeFlags);
		ZP_ASSERT(m_pxShape, "Failed to create pxShape");

		offsetTransform[0] = glm::vec4(glm::normalize(glm::vec3(offsetTransform[0])), offsetTransform[0].w);
		offsetTransform[1] = glm::vec4(glm::normalize(glm::vec3(offsetTransform[1])), offsetTransform[1].w);
		offsetTransform[2] = glm::vec4(glm::normalize(glm::vec3(offsetTransform[2])), offsetTransform[2].w);

		physx::PxTransform t = physx::PxTransform(*(physx::PxMat44*)(&offsetTransform));

		m_pxShape->setLocalPose(t);
	}

	Shape::Shape(physx::PxShape* pxShape) {
		m_pxShape = pxShape;
	}

	Shape::~Shape() {}

	void Shape::release() {
		m_pxShape->release();
	}

	void Shape::setGeometry(PhysicsGeometry& geometry) {
		m_pxShape->setGeometry(*geometry.m_pxGeometry);
	}

	physx::PxShape* Shape::getPxShape() {
		return m_pxShape;
	}
}