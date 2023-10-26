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
        static std::vector<Light> all;

        glm::vec3 m_color;

        friend class PBRenderer;
    };
}

