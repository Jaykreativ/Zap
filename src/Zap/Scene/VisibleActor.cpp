#include "Zap/Scene/VisibleActor.h"

namespace Zap {
    VisibleActor::VisibleActor() {}
    VisibleActor::~VisibleActor() {}

    void VisibleActor::setModel(Model& model) {
        m_model = &model;
    }

    Model* VisibleActor::getModel() {
        return m_model;
    }
}