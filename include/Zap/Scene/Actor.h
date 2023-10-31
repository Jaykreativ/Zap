#pragma once

#include "Zap/Zap.h"
#include "Zap/Scene/Component.h"
#include "Zap/Scene/Mesh.h"
#include "glm.hpp"

namespace Zap {
    class Actor
    {
    public:
        Actor();
        ~Actor();

        void translate(glm::vec3 pos);
        void translate(float x, float y, float z);

        void setPos(glm::vec3 pos);
        void setPos(float x, float y, float z);

        void rotateX(float angle);
        void rotateY(float angle);
        void rotateZ(float angle);
        void rotate(float angle, glm::vec3 axis);

        void setScale(glm::vec3 scale);
        void setScale(float x, float y, float z);

        void setTransform(glm::mat4& transform);

        glm::vec3 getPos();

        glm::mat4 getTransform();

        glm::mat4 m_transform = glm::mat4(1);

        void addMeshComponent(Mesh* pMesh);
        void addLightComponent(glm::vec3 color);
        void addCameraComponent(glm::vec3 offset);

        std::vector<uint32_t> getComponentIDs(ComponentType type);

        Component* getComponent(ComponentType type, uint32_t index);

    private:
        struct ComponentAccess
        {
            ComponentType type;
            uint32_t id;
        };
        std::vector<ComponentAccess> m_components;
    };
}

