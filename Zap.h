#pragma once

#include "VulkanFramework.h"

namespace Zap {
    namespace GlobalSettings {
        VkFormat getColorFormat();
    }

    void init(const char* applicationName);

    void terminate();
}