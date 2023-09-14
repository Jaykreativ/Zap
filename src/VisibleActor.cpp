#include "VisibleActor.h"

namespace Zap {
    VisibleActor::VisibleActor() {}
    VisibleActor::~VisibleActor() {}

    void VisibleActor::setVertexArray(Vertex* data, uint32_t size) {
        m_vertexArray.resize(size);
        m_vertexArray.assign(data, data+size);
    }
    void VisibleActor::setIndexArray(uint32_t* data, uint32_t size) {
        m_indexArray.resize(size);
        m_indexArray.assign(data, data+size); 
    }

    const std::vector<Vertex>& VisibleActor::getVertexArray() {
        return m_vertexArray;
    }
    const std::vector<uint32_t>& VisibleActor::getIndexArray() {
        return m_indexArray;
    }
}