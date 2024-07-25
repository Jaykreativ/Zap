#pragma once

#include "Zap/UUID.h"
#include "Zap/Vertex.h"
#include "VulkanFramework.h"

namespace Zap {
    struct MeshData {
        glm::mat4 m_transform = glm::mat4(1);
        vk::Buffer m_vertexBuffer = vk::Buffer();
        vk::Buffer m_indexBuffer = vk::Buffer();
    };

    class Mesh
    {
    public:
        Mesh();
        Mesh(UUID handle);
        ~Mesh();

        void init();

        void load(uint32_t vertexCount, Vertex* pVertices, uint32_t indexCount, uint32_t* pIndices);
        void load(std::vector<Vertex> vertexArray, std::vector<uint32_t> indexArray);

        void destroy();

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

