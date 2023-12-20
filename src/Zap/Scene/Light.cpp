#include "Zap/Scene/Light.h"

namespace Zap {
    std::vector<Light> Light::all;

    Light::Light(Actor* pActor, glm::vec3 color) : Component(pActor), m_color(color)
    {
        m_id = all.size();
        all.push_back(*this);
    }

    Light::Light(Actor* pActor) : Light(pActor, glm::vec3(0, 0, 0))
    {}

    void Light::setColor(glm::vec3 color) {
        m_color = color;
    }

    glm::vec3 Light::getColor() {
        return m_color;
    }
}