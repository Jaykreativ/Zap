#include "Model.h"

namespace Zap {
	Model::Model(){}

	Model::~Model() {
		if (!m_isInit) return;
		m_isInit = false;

		m_vertexBuffer.~Buffer();
		m_indexBuffer.~Buffer();

		m_commandBufferCount = 0;
		delete[] m_commandBuffers;
	}

	void Model::init(uint32_t commandBufferCount) {
		if (m_isInit) return;
		m_isInit = true;

		m_commandBufferCount = commandBufferCount;
		m_commandBuffers = new vk::CommandBuffer[m_commandBufferCount];
		for (int i = 0; i < m_commandBufferCount; i++) {
			m_commandBuffers[i].allocate();
		}
	}

	void Model::recordCommandBuffers(vk::RenderPass& renderPass, vk::Framebuffer* framebuffers, VkRect2D renderArea, vk::Pipeline& pipeline, VkDescriptorSet descriptorSet) {
		for (int i = 0; i < m_commandBufferCount; i++) {
			vk::CommandBuffer& cmd = m_commandBuffers[i];
			cmd.begin(VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT);

			VkRenderPassBeginInfo renderPassBeginInfo;
			renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
			renderPassBeginInfo.pNext = nullptr;
			renderPassBeginInfo.renderPass = renderPass;
			renderPassBeginInfo.framebuffer = framebuffers[i];
			renderPassBeginInfo.renderArea = renderArea;
			std::vector<VkClearValue> clearValues{
				{ 0.1, 0.1, 0.1, 1 }
			};
			renderPassBeginInfo.clearValueCount = clearValues.size();
			renderPassBeginInfo.pClearValues = clearValues.data();

			vkCmdBeginRenderPass(cmd, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

			vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);

			VkDeviceSize offsets[] = { 0 };
			VkBuffer vertexBuffer = m_vertexBuffer.getVkBuffer();
			vkCmdBindVertexBuffers(cmd, 0, 1, &vertexBuffer, offsets);
			vkCmdBindIndexBuffer(cmd, m_indexBuffer, 0, VK_INDEX_TYPE_UINT32);

			vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.getVkPipelineLayout(), 0, 1, &descriptorSet, 0, nullptr);

			vkCmdDrawIndexed(cmd, m_indexBuffer.getSize()/sizeof(uint32_t), 1, 0, 0, 0);

			vkCmdEndRenderPass(cmd);

			cmd.end();
		}
	}

	void Model::load(uint32_t vertexCount, Vertex* pVertices, uint32_t indexCount, uint32_t* pIndices) {
		m_vertexBuffer = vk::Buffer(vertexCount * sizeof(Vertex), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);
		m_vertexBuffer.init(); m_vertexBuffer.allocate(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
		m_vertexBuffer.uploadData(m_vertexBuffer.getSize(), pVertices);

		m_indexBuffer = vk::Buffer(indexCount * sizeof(uint32_t), VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);
		m_indexBuffer.init(); m_indexBuffer.allocate(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
		m_indexBuffer.uploadData(m_indexBuffer.getSize(), pIndices);
	}

	void Model::load(std::vector<Vertex> vertexArray, std::vector<uint32_t> indexArray) {
		load(vertexArray.size(), vertexArray.data(), indexArray.size(), indexArray.data());
	}

	void Model::load(const char* modelPath) {
		Assimp::Importer importer;
		const aiScene* scene = importer.ReadFile(modelPath, 0);
		std::cout << modelPath << " -> NumMeshes: " << scene->mNumMeshes << "\n";
	}
}