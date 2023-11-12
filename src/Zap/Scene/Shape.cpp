#include "Zap/Scene/Shape.h"

namespace Zap {
    PhysicsMaterial::PhysicsMaterial(uint32_t staticFriction, uint32_t dynamicFriction, uint32_t restitution) {
        auto base = Base::getBase();
        m_pxMaterial = base->m_pxPhysics->createMaterial(staticFriction, dynamicFriction, restitution);
    }

    PhysicsMaterial::~PhysicsMaterial() {
        m_pxMaterial->release();
    }

    BoxGeometry::BoxGeometry(glm::vec3 size) {
        m_pxGeometry = new physx::PxBoxGeometry(size.x, size.y, size.z);
    }

    BoxGeometry::~BoxGeometry() {
        delete m_pxGeometry;
    }

    Shape::Shape(PhysicsGeometry geometry, PhysicsMaterial material, bool isExclusive) {
        auto base = Base::getBase();
        m_pxShape = base->m_pxPhysics->createShape(*geometry.m_pxGeometry, *material.m_pxMaterial, isExclusive);
    }

    Shape::~Shape() {
        m_pxShape->release();
    }
}