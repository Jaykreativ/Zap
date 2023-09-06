#pragma once

#include "VulkanFramework.h"

namespace Zap {
    namespace GlobalSettings {
        VkFormat getColorFormat();
    }

    vk::DescriptorPool& getDescriptorPool();

    void init(const char* applicationName);

    void terminate();
}