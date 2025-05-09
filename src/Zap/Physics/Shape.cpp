#include "Zap/Physics/Shape.h"

#include "glm/gtc/quaternion.hpp"

namespace Zap {
	PhysicsMaterial::PhysicsMaterial(float staticFriction, float dynamicFriction, float restitution) {
		auto base = Base::getBase();
		m_pxMaterial = base->m_pxPhysics->createMaterial(staticFriction, dynamicFriction, restitution);
	}

	PhysicsMaterial::PhysicsMaterial(physx::PxMaterial* pxMaterial)
		: m_pxMaterial(pxMaterial)
	{}

	PhysicsMaterial::~PhysicsMaterial() {}

	void PhysicsMaterial::release() {
		m_pxMaterial->release();
	}
	
	float PhysicsMaterial::getDynamicFriction() const {
		return m_pxMaterial->getDynamicFriction();
	}

	float PhysicsMaterial::getStaticFriction() const {
		return m_pxMaterial->getStaticFriction();
	}

	float PhysicsMaterial::getRestitution() const {
		return m_pxMaterial->getRestitution();
	}

	Shape::Shape(const PhysicsGeometry& geometry, PhysicsMaterial material, bool isExclusive, glm::mat4 offsetTransform, physx::PxShapeFlags shapeFlags) {
		auto base = Base::getBase();

		m_pxShape = base->m_pxPhysics->createShape(geometry, &material.m_pxMaterial, 1, isExclusive, shapeFlags);
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

	void Shape::setGeometry(const PhysicsGeometry& geometry) {
		m_pxShape->setGeometry(geometry);
	}

	std::unique_ptr<PhysicsGeometry> Shape::getGeometry() {
		const auto& geometry = m_pxShape->getGeometry();
		switch (geometry.getType())
		{
		case physx::PxGeometryType::eSPHERE:
			return std::make_unique<SphereGeometry>(static_cast<const physx::PxSphereGeometry&>(m_pxShape->getGeometry()));
		case physx::PxGeometryType::eCAPSULE:
			return std::make_unique<CapsuleGeometry>(static_cast<const physx::PxCapsuleGeometry&>(m_pxShape->getGeometry()));
		case physx::PxGeometryType::eBOX:
			return std::make_unique<BoxGeometry>(static_cast<const physx::PxBoxGeometry&>(m_pxShape->getGeometry()));
		case physx::PxGeometryType::ePLANE:
			return std::make_unique<PlaneGeometry>(static_cast<const physx::PxPlaneGeometry&>(m_pxShape->getGeometry()));
		case physx::PxGeometryType::eCONVEXMESH:
			return std::make_unique<ConvexMeshGeometry>(static_cast<const physx::PxConvexMeshGeometry&>(m_pxShape->getGeometry()));
		default: {
			ZP_WARN(false, "Shape::getGeometry unknown geometry type");
			return nullptr;
		}
		}
	}

	bool Shape::isExclusive() {
		return m_pxShape->isExclusive();
	}

	void Shape::setLocalPose(glm::mat4 transform) {
		m_pxShape->setLocalPose(PxUtils::glmMat4ToTransform(transform));
	}

	void Shape::setLocalPosition(glm::vec3 pos) {
		auto pxTransform = m_pxShape->getLocalPose();
		pxTransform.p = PxUtils::glmVec3toVec3(pos);
		m_pxShape->setLocalPose(pxTransform);
	}

	void Shape::setLocalRotation(glm::quat quat) {
		auto pxTransform = m_pxShape->getLocalPose();
		pxTransform.q = PxUtils::glmQuatToQuat(quat);
		m_pxShape->setLocalPose(pxTransform);
	}

	glm::mat4 Shape::getLocalPose() {
		auto pxTransform = m_pxShape->getLocalPose();
		return PxUtils::transformToGlmMat4(pxTransform);
	}

	glm::vec3 Shape::getLocalPosition() {
		auto pxTransform = m_pxShape->getLocalPose();
		return PxUtils::vec3ToGlmVec3(pxTransform.p);
	}

	glm::quat Shape::getLocalRotation() {
		auto pxTransform = m_pxShape->getLocalPose();
		return PxUtils::quatToGlmQuat(pxTransform.q);
	}

	PhysicsMaterial Shape::getMaterial() {
		size_t nbMaterials = m_pxShape->getNbMaterials();
		ZP_ASSERT(nbMaterials > 0, "Shape has no Materials");
		physx::PxMaterial* pxMaterial;
		m_pxShape->getMaterials(&pxMaterial, 1); // get the one Material
		return pxMaterial;
	}

	physx::PxShape* Shape::getPxShape() {
		return m_pxShape;
	}
}