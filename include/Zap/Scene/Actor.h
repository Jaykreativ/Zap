#pragma once

#include "Zap/Zap.h"
#include "Zap/Scene/Component.h"
#include "Zap/Scene/Mesh.h"
#include "glm.hpp"

namespace Zap {
    class Transform;
    class MeshComponent;
    class Light;
    class Camera;

    class Actor
    {
    public:
        void addTransform(glm::mat4 transform);
        void addMesh(Mesh* pMesh);
        void addLight(glm::vec3 color);
        void addCamera(glm::vec3 offset);

        glm::mat4 getTransform();

        void setTransform(glm::mat4 transform);

        std::vector<uint32_t> getComponentIDs(ComponentType type);

        Component* getComponent(ComponentType type, uint32_t index);

        Transform* getTransformComponent(uint32_t index);

        MeshComponent* getMeshComponent(uint32_t index);

        Light* getLightComponent(uint32_t index);

        Camera* getCameraComponent(uint32_t index);

    private:
        struct ComponentAccess
        {
            ComponentType type;
            uint32_t id;
        };
        std::vector<ComponentAccess> m_components;
    };
}

