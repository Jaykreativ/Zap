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

    class Shape
    {
    public:
        Shape(PhysicsGeometry geometry, PhysicsMaterial material, bool isExclusive);
        ~Shape();

    private:
        physx::PxShape* m_pxShape;

        friend class PhysicsComponent;
    };
}

