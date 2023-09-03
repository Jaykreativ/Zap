#pragma once

#include "Zap.h"

class Vertex
{
public:
	Vertex(glm::vec3 pos)
		: m_pos(pos)
	{}
	~Vertex() {}

	glm::vec3 m_pos;

	static uint32_t getVertexInputAttributeDescriptionCount() {
		return 1;
	}

	static std::vector<VkVertexInputAttributeDescription> getVertexInputAttributeDescriptions() {
		VkVertexInputAttributeDescription posAttributeDescription;
		posAttributeDescription.location = 0;
		posAttributeDescription.binding = 0;
		posAttributeDescription.format = VK_FORMAT_R32G32B32A32_SFLOAT;
		posAttributeDescription.offset = offsetof(Vertex, m_pos);

		std::vector<VkVertexInputAttributeDescription> attributeDescriptions = {
			posAttributeDescription
		};
	}

	static VkVertexInputBindingDescription getVertexInputBindingDescription() {
		VkVertexInputBindingDescription bindingDescription;
		bindingDescription.binding = 0;
		bindingDescription.stride = sizeof(Vertex);
		bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
	}
};

