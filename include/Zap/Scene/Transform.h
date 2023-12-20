#pragma once

#include "Zap/Scene/Component.h"
#include "glm.hpp"

namespace Zap {
    class Transform : public Component {
    public:
        Transform(glm::mat4 transform, Actor* pActor);

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
    
    private:
        glm::mat4 m_transform = glm::mat4(1);

        static std::vector<Transform> all;

        friend class Actor;
    };
}

