#include "Model.h"

namespace Zap {
    Model::Model(){}
    Model::~Model() {}

    void Model::init(uint32_t commandBufferCount) {
        m_commandBufferCount = commandBufferCount;
        m_commandBuffers = new vk::CommandBuffer[m_commandBufferCount];
        for (int i = 0; i < m_commandBufferCount; i++) {
            m_commandBuffers[i].allocate();
        }
    }

    void Model::recordCommandBuffers(vk::RenderPass& renderPass) {
        for (int i = 0; i < m_commandBufferCount; i++) {
            vk::CommandBuffer& cmd = m_commandBuffers[i];
            cmd.begin(VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT);

            VkRenderPassBeginInfo renderPassBeginInfo;
            renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
            renderPassBeginInfo.pNext = nullptr;
            renderPassBeginInfo.renderPass = renderPass;
            renderPassBeginInfo.framebuffer = ;
            renderPassBeginInfo.renderArea = ;
            renderPassBeginInfo.clearValueCount = ;
            renderPassBeginInfo.pClearValues = ;

            cmd.end();
        }
    }

    void Model::load(std::vector<Vertex> vertexArray, std::vector<uint32_t> indexArray) {
        m_vertexBuffer = vk::Buffer(vertexArray.size() * sizeof(Vertex), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);
        m_vertexBuffer.init(); m_vertexBuffer.allocate(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
        m_vertexBuffer.uploadData(m_vertexBuffer.getSize(), vertexArray.data());

        m_indexBuffer = vk::Buffer(indexArray.size() * sizeof(uint32_t), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);
        m_indexBuffer.init(); m_indexBuffer.allocate(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
        m_indexBuffer.uploadData(m_indexBuffer.getSize(), indexArray.data());
    }
}