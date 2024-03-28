#pragma once

#include "Zap/Zap.h"
#include "Zap/Scene/Model.h"
#include <vector>

class aiMesh;// forwarded decleration
class aiTexture;

namespace Zap {
	class ModelLoader
	{
	public:
		ModelLoader();

		~ModelLoader();

		enum LoadFlags {
			eTintTextures = 0x1
		};

		Model load(const char* modelPath, uint32_t flags = 0);

		Model loadFromMemory(uint32_t vertexCount, Vertex* pVertices, uint32_t indexCount, uint32_t* pIndices);
		Model loadFromMemory(std::vector<Vertex> vertexArray, std::vector<uint32_t> indexArray);

		/*
		* Loads a texture into the texture system.
		* Returns a handle to the texture in the system.
		* Needs to have a layout of R8G8B8A8
		*/
		uint32_t loadTexture(void* data, uint32_t width, uint32_t height);

		uint32_t loadTexture(const char* texturePath);

		uint32_t loadTexture(const aiTexture* texture);

	private:
		uint32_t loadMesh(aiMesh* aMesh);
	};
}

