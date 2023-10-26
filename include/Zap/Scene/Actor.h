#pragma once

#include "Zap/Zap.h"
#include "Zap/Scene/Component.h"
#include "Zap/Scene/MeshComponent.h"
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

        void addMeshComponent(Mesh* pMesh){
            uint32_t id;
            id = MeshComponent(this, pMesh).getID();
            m_components.push_back(ComponentAccess{ COMPONENT_TYPE_MESH, id });
        }

        std::vector<uint32_t> getComponents(ComponentType type);

    private:
        struct ComponentAccess
        {
            ComponentType type;
            uint32_t id;
        };
        std::vector<ComponentAccess> m_components;
    };
}

