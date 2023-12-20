#include "Zap/Scene/Shape.h"

namespace Zap {
    PhysicsMaterial::PhysicsMaterial(uint32_t staticFriction, uint32_t dynamicFriction, uint32_t restitution) {
        auto base = Base::getBase();
        m_pxMaterial = base->m_pxPhysics->createMaterial(staticFriction, dynamicFriction, restitution);
    }

    PhysicsMaterial::~PhysicsMaterial() {
        //TODO clean up materials
        //m_pxMaterial->release();
    }

    BoxGeometry::BoxGeometry(glm::vec3 size) {
        m_pxGeometry = new physx::PxBoxGeometry(size.x, size.y, size.z);
    }

    BoxGeometry::~BoxGeometry() {
        delete m_pxGeometry;
    }

    PlaneGeometry::PlaneGeometry() {
        m_pxGeometry = new physx::PxPlaneGeometry();
    }

    PlaneGeometry::~PlaneGeometry() {
        delete m_pxGeometry;
    }

    Shape::Shape(PhysicsGeometry geometry, PhysicsMaterial material, bool isExclusive, glm::mat4 offsetTransform) {
        auto base = Base::getBase();
        if (!m_hasShape) {
            m_pxShape = base->m_pxPhysics->createShape(*geometry.m_pxGeometry, &material.m_pxMaterial, isExclusive);
            m_hasShape = true;

            offsetTransform[0] = glm::vec4(glm::normalize(glm::vec3(offsetTransform[0])), offsetTransform[0].w);
            offsetTransform[1] = glm::vec4(glm::normalize(glm::vec3(offsetTransform[1])), offsetTransform[1].w);
            offsetTransform[2] = glm::vec4(glm::normalize(glm::vec3(offsetTransform[2])), offsetTransform[2].w);

            physx::PxTransform t = physx::PxTransform(*(physx::PxMat44*)(&offsetTransform));

            m_pxShape->setLocalPose(t);
        }
    }

    Shape::~Shape() {
        if (m_hasShape) {
            m_pxShape->release();
            m_hasShape = false;
        }
    }

    Shape::Shape(Shape& shape) {
        m_hasShape = false;
        m_pxShape = shape.m_pxShape;
    }
}