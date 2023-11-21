#pragma once
#include "Zap/Zap.h"
#include "Zap/Vertex.h"
#include "Zap/Scene/Material.h"
#include "assimp/Importer.hpp"
#include "assimp/scene.h"
#include "assimp/postprocess.h"

namespace Zap {
    class Mesh
    {
    public:
        Mesh();
        ~Mesh();

        void init();

        void load(uint32_t vertexCount, Vertex* pVertices, uint32_t indexCount, uint32_t* pIndices);
        void load(std::vector<Vertex> vertexArray, std::vector<uint32_t> indexArray);
        void load(const char* modelPath);

        vk::CommandBuffer* getCommandBuffer(int index);

        vk::Buffer* getVertexBuffer();

        vk::Buffer* getIndexbuffer();

    private:
        bool m_isInit = false;

        uint32_t m_id;

        std::vector<vk::CommandBuffer> m_commandBuffers;
        vk::Buffer m_vertexBuffer = vk::Buffer();
        vk::Buffer m_indexBuffer = vk::Buffer();

        static std::vector<Mesh> all;

        friend class Base;
        friend class Renderer;
        friend class PBRenderer;
    };
}

