#pragma once

#include "Zap/UUID.h"
#include "Zap/Vertex.h"
#include "VulkanFramework.h"

namespace Zap {
	struct MeshData {
		glm::mat4 m_transform = glm::mat4(1);
		vk::Buffer m_vertexBuffer = vk::Buffer();
		vk::Buffer m_indexBuffer = vk::Buffer();
		glm::vec3 m_boundMin = { 0, 0, 0 };
		glm::vec3 m_boundMax = { 0, 0, 0 };
	};

	class Mesh
	{
	public:
		Mesh();
		Mesh(UUID handle);
		~Mesh();

		void load(uint32_t vertexCount, Vertex* pVertices, uint32_t indexCount, uint32_t* pIndices);
		void load(std::vector<Vertex> vertexArray, std::vector<uint32_t> indexArray);

		void destroy();
		static void destroy(MeshData* data);

		// removes asset from assetLibrary and destroys it
		// only works correctly with runtime generated assets
		void remove();

		void setBoundingBox(glm::vec3 boundMin, glm::vec3 boundMax);
		
		bool exists() const;

		UUID getHandle();

		const glm::mat4* getTransform() const;

		const vk::Buffer* getVertexBuffer() const;

		const vk::Buffer* getIndexBuffer() const;

	private:
		UUID m_handle;

		friend class Base;
		friend class Scene;
		friend class Renderer;
		friend class PBRenderer;
		friend class RaytracingRenderer;
		friend class PathTracer;
		friend class ModelLoader;
	};
}

