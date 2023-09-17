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

        void recordCommandBuffers(vk::RenderPass& renderPass, vk::Framebuffer* framebuffers, VkRect2D renderArea, vk::Pipeline& pipeline, VkDescriptorSet descriptorSet);

        void load(std::vector<Vertex> vertexArray, std::vector<uint32_t> indexArray);

        vk::CommandBuffer* getCommandBuffer(int index) {
            return &m_commandBuffers[index];
        }

    private:
        bool m_isInit = false;

        uint32_t m_commandBufferCount = 0;
        vk::CommandBuffer* m_commandBuffers = nullptr;
        vk::Buffer m_vertexBuffer = vk::Buffer();
        vk::Buffer m_indexBuffer = vk::Buffer();
    };
}

