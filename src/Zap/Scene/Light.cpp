#include "Zap/Scene/Light.h"

namespace Zap {
    Light::Light(){}
    Light::~Light() {}

    void Light::setColor(glm::vec3 color) {
        m_color = color;
    }

    glm::vec3 Light::getColor() {
        return m_color;
    }
}