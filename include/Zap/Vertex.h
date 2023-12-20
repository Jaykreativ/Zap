#pragma once

#include "Zap.h"
#include "glm.hpp"

class Vertex
{
public:
	Vertex() {}
<<<<<<<< HEAD:include/Vertex.h
	Vertex(glm::vec3 pos)
		: m_pos(pos)
========
	Vertex(glm::vec3 pos, glm::vec3 normal)
		: m_pos(pos), m_normal(normal)
>>>>>>>> save:include/Zap/Vertex.h
	{}
	~Vertex() {}

	glm::vec3 m_pos;
	glm::vec3 m_normal;

	static uint32_t getVertexInputAttributeDescriptionCount() {
		return 2;
	}

	static std::vector<VkVertexInputAttributeDescription> getVertexInputAttributeDescriptions() {
		VkVertexInputAttributeDescription posAttributeDescription;
		posAttributeDescription.location = 0;
		posAttributeDescription.binding = 0;
		posAttributeDescription.format = VK_FORMAT_R32G32B32_SFLOAT;
		posAttributeDescription.offset = offsetof(Vertex, m_pos);

		VkVertexInputAttributeDescription normalAttributeDescription;
		normalAttributeDescription.location = 1;
		normalAttributeDescription.binding = 0;
		normalAttributeDescription.format = VK_FORMAT_R32G32B32_SFLOAT;
		normalAttributeDescription.offset = offsetof(Vertex, m_normal);

		std::vector<VkVertexInputAttributeDescription> attributeDescriptions = {
			posAttributeDescription,
			normalAttributeDescription
		};
		return attributeDescriptions;
	}

	static VkVertexInputBindingDescription getVertexInputBindingDescription() {
		VkVertexInputBindingDescription bindingDescription;
		bindingDescription.binding = 0;
		bindingDescription.stride = sizeof(Vertex);
		bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
		return bindingDescription;
	}
};

