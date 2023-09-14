#pragma once
#include "Zap.h"
#include "Vertex.h"

namespace Zap {
    class Model
    {
    public:
        Model();
        ~Model();

        void init(uint32_t commandBufferCount);

        void recordCommandBuffers();

        void load(std::vector<Vertex> vertexArray, std::vector<uint32_t> indexArray);

    private:
        uint32_t m_commandBufferCount = 0;
        vk::CommandBuffer* m_commandBuffers = nullptr;
        vk::Buffer m_vertexBuffer = vk::Buffer();
        vk::Buffer m_indexBuffer = vk::Buffer();
    };
}

