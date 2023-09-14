#include "Actor.h"

namespace Zap {
    Actor::Actor(){}
    Actor::~Actor(){}

    void Actor::setTransform(glm::mat4 transform) {
        m_tranform = transform;
    }

    glm::mat4* Actor::getTransform() {
        return &m_tranform;
    }
}