#pragma once

#include "Zap/Zap.h"
#include <vector>

namespace Zap {
    class ModelLoader
    {
    public:
        ModelLoader();

        ~ModelLoader();

        std::vector<uint32_t> load(const char* modelPath);
    };
}

