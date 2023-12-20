#pragma once

#include "Zap/Zap.h"
#include <vector>

class aiMesh;

namespace Zap {
    class ModelLoader
    {
    public:
        ModelLoader();

        ~ModelLoader();

        std::vector<uint32_t> load(const char* modelPath);

    private:
        uint32_t loadMesh(aiMesh* aMesh);
    };
}

