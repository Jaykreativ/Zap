#pragma once
#include "glm.hpp"

namespace Zap {
    class Actor
    {
    public:
        Actor();
        ~Actor();

        void setTransform(glm::mat4 transform);

        glm::mat4* getTransform();

    private:
        glm::mat4 m_tranform = glm::mat4(1);
    };
}

