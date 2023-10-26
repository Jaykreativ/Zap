#pragma once

#include "Zap/Zap.h"
#include "Zap/Scene/Actor.h"

namespace Zap {
    class Light : public Actor
    {
    public:
        Light();
        ~Light();

        void setColor(glm::vec3 color);

        glm::vec3 getColor();

    private:
        glm::vec3 m_color;
    };
}

