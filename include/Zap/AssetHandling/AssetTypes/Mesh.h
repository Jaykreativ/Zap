#pragma once

#include "Zap/UUID.h"
#include "Zap/AssetHandling/Asset.h"
#include "Zap/Vertex.h"
#include "VulkanFramework.h"

namespace Zap {
	class Mesh : public Asset {
	public:
		Mesh(
			glm::mat4  transform,
			vk::Buffer vertexBuffer,
			vk::Buffer indexBuffer,
			glm::vec3  boundMax,
			glm::vec3  boundMin
		);
		~Mesh();

		const glm::vec3& getBoundMin() const;
		const glm::vec3& getBoundMax() const;

		const glm::mat4& getTransform() const;

		const vk::Buffer& getVertexBuffer() const;

		const vk::Buffer& getIndexBuffer() const;

	private:
		glm::mat4 m_transform = glm::mat4(1);
		vk::Buffer m_vertexBuffer = vk::Buffer();
		vk::Buffer m_indexBuffer = vk::Buffer();
		glm::vec3 m_boundMin = { 0, 0, 0 };
		glm::vec3 m_boundMax = { 0, 0, 0 };
	};
}

