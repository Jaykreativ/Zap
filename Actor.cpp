#include "Actor.h"

namespace Zap {
    void Actor::setTransform(glm::mat4 transform) {
        m_tranform = transform;
    }

    glm::mat4 Actor::getTransform() {
        return m_tranform;
    }
}