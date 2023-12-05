#pragma once

#include "Zap/Zap.h"
#include "glm.hpp"

namespace Zap {
    class PhysicsMaterial {
    public:
        PhysicsMaterial(uint32_t staticFriction, uint32_t dynamicFriction, uint32_t restitution);
        ~PhysicsMaterial();

    private:
        physx::PxMaterial* m_pxMaterial;

        friend class Shape;
    };

    class PhysicsGeometry {
    public:
        PhysicsGeometry() = default;

    protected:
        physx::PxGeometry* m_pxGeometry;

    private:

        friend class Shape;
    };

    class BoxGeometry : public PhysicsGeometry {
    public:
        BoxGeometry(glm::vec3 size);
        ~BoxGeometry();
    };

    class PlaneGeometry : public PhysicsGeometry {
    public:
        PlaneGeometry();
        ~PlaneGeometry();
    };

    class Shape
    {
    public:
        Shape(PhysicsGeometry geometry, PhysicsMaterial material, bool isExclusive, glm::mat4 offsetTransform = glm::mat4(1));
        ~Shape();
        Shape(Shape& shape);

    private:
        bool m_hasShape = false;

        physx::PxShape* m_pxShape;

        friend class PhysicsComponent;
        friend class RigidBodyComponent;
        friend class RigidDynamicComponent;
        friend class RigidStaticComponent;
    };
}

