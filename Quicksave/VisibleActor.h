#pragma once

#include "Zap/Scene/Actor.h"
#include "Zap/Vertex.h"
#include "Zap/Scene/Mesh.h"

namespace Zap {
    class VisibleActor : public Actor {
    public:
        VisibleActor();
        ~VisibleActor();

        void setModel(Mesh& model);

        Mesh* getModel();

        glm::vec3 m_color = { 1, 1, 1 };// TODO Texture

    private:
        Mesh* m_model;
    };
}

