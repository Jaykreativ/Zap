#pragma once

#include "Actor.h"
#include "Vertex.h"

namespace Zap {
    class VisibleActor : public Actor {
    public:
        VisibleActor();
        ~VisibleActor();

        void setVertexArray(Vertex* data, uint32_t size);
        void setIndexArray(uint32_t* data, uint32_t size);

        const std::vector<Vertex>& getVertexArray();
        const std::vector<uint32_t>& getIndexArray();

    private:
        std::vector<Vertex> m_vertexArray;
        std::vector<uint32_t> m_indexArray;
    };
}

