#pragma once

#include "Actor.h"
#include "Vertex.h"
#include "Model.h"

namespace Zap {
    class VisibleActor : public Actor {
    public:
        VisibleActor();
        ~VisibleActor();

        void setModel(Model& model);

        Model* getModel();

    private:
        Model* m_model;
    };
}

