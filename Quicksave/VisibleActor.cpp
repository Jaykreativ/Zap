#include "Zap/Scene/VisibleActor.h"

namespace Zap {
    VisibleActor::VisibleActor() {}
    VisibleActor::~VisibleActor() {}

    void VisibleActor::setModel(Mesh& model) {
        m_model = &model;
    }

    Mesh* VisibleActor::getModel() {
        return m_model;
    }
}