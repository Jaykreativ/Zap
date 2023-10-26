#pragma once

#include "Zap/Scene/Actor.h"
#include "Zap/Vertex.h"
#include "Model.h"

namespace Zap {
    class VisibleActor : public Actor {
    public:
        VisibleActor();
        ~VisibleActor();

        void setModel(Model& model);

        Model* getModel();

        glm::vec3 m_color = { 1, 1, 1 };// TODO Texture

    private:
        Model* m_model;
    };
}

