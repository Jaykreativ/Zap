#pragma once

#include "Zap/Zap.h"
#include "Zap/Scene/Component.h"
#include "Zap/Scene/Mesh.h"
#include "Zap/Scene/Shape.h"
#include "glm.hpp"

namespace Zap {
    class Transform;
    class MeshComponent;
    class PhysicsComponent;
    class Light;
    class Camera;

    class Actor
    {
    public:
        Actor();
        ~Actor();

        bool addTransform(glm::mat4 transform);
        bool addMesh(Mesh* pMesh);
        bool addPhysics(PhysicsType type, Shape shape);
        bool addLight(glm::vec3 color);
        bool addCamera(glm::vec3 offset);

        glm::mat4 getTransform();

        void setTransform(glm::mat4 transform);

        std::vector<uint32_t> getComponentIDs(ComponentType type);

        Component* getComponent(ComponentType type, uint32_t index);

        Transform* getTransformComponent();

        MeshComponent* getMeshComponent(uint32_t index);

        PhysicsComponent* getPhysicsComponent(uint32_t index);

        Light* getLightComponent(uint32_t index);

        Camera* getCameraComponent(uint32_t index);

    private:
        enum TransformState {
            TRANSFORM_STATE_NONE = 0,
            TRANSFORM_STATE_COMPONENT = 1,
            TRANSFORM_STATE_PHYSICS = 2
        };

        TransformState m_transformState = TRANSFORM_STATE_NONE;

        std::vector<ComponentAccess> m_components;
    };
}

