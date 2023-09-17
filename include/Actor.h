#pragma once
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

        void setTransform(glm::mat4& transform);

        glm::vec3 getPos();

        glm::mat4 getTransform();

    protected:
        glm::mat4 m_transform = glm::mat4(1);
    };
}

