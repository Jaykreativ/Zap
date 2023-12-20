#pragma once

#include "Zap/Zap.h"
#include "Zap/Scene/Component.h"
#include "glm.hpp"

namespace Zap {

    class Light : public Component
    {
    public:
        Light(Actor* pActor, glm::vec3 color);
        Light(Actor* pActor);

        void setColor(glm::vec3 color);

        glm::vec3 getColor();

    private:
        glm::vec3 m_color;

        static std::vector<Light> all;

        friend class Actor;
        friend class PBRenderer;
    };
}

