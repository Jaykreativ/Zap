#pragma once

#include "glm.hpp"
#include "VulkanFramework.h"

class Vertex
{
public:
	Vertex() {}
	Vertex(glm::vec3 pos, glm::vec3 normal)
		: pos(pos), normal(normal)
	{}
	~Vertex() {}

	glm::vec3 pos;
	glm::vec2 texCoords;
	glm::vec3 normal;

	static uint32_t getVertexInputAttributeDescriptionCount() {
		return 3;
	}

	static std::vector<VkVertexInputAttributeDescription> getVertexInputAttributeDescriptions() {
		VkVertexInputAttributeDescription posAttributeDescription{};
		posAttributeDescription.location = 0;
		posAttributeDescription.binding = 0;
		posAttributeDescription.format = VK_FORMAT_R32G32B32_SFLOAT;
		posAttributeDescription.offset = offsetof(Vertex, pos);

		VkVertexInputAttributeDescription texCoordsAttributeDescripton{};
		texCoordsAttributeDescripton.location = 1;
		texCoordsAttributeDescripton.binding = 0;
		texCoordsAttributeDescripton.format = VK_FORMAT_R32G32_SFLOAT;
		texCoordsAttributeDescripton.offset = offsetof(Vertex, texCoords);

		VkVertexInputAttributeDescription normalAttributeDescription{};
		normalAttributeDescription.location = 2;
		normalAttributeDescription.binding = 0;
		normalAttributeDescription.format = VK_FORMAT_R32G32B32_SFLOAT;
		normalAttributeDescription.offset = offsetof(Vertex, normal);

		std::vector<VkVertexInputAttributeDescription> attributeDescriptions = {
			posAttributeDescription,
			texCoordsAttributeDescripton,
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

